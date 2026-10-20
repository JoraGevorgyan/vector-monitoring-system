#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <thread>
#include <type_traits>
#include <vector>

#include "common/Vector3D.hpp"
#include "shared_vector/SharedVector.hpp"

namespace {

using concurrent::SharedVector;

// Vector3D exposes only is_in_range(), so a vector is identified by its
// magnitude. All magnitudes used here are exact in floating point.
bool has_magnitude(const common::Vector3D& v, common::el_t magnitude)
{
	return v.is_in_range(magnitude, magnitude);
}

// Chosen so that mixing components of the two vectors (a torn read) never
// yields magnitude 5 or 7.
constexpr common::el_t a_x = 3.;
constexpr common::el_t a_y = 4.;
constexpr common::el_t a_z = 0.;
constexpr common::el_t a_magnitude = 5.;

constexpr common::el_t b_x = 2.;
constexpr common::el_t b_y = 3.;
constexpr common::el_t b_z = 6.;
constexpr common::el_t b_magnitude = 7.;

constexpr std::size_t thread_count = 8;
constexpr std::size_t iterations = 20000;

void wait_for(const std::atomic<bool>& go)
{
	while (!go.load(std::memory_order_acquire)) {
		std::this_thread::yield();
	}
}

} // namespace

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

TEST(SharedVectorSingleton, RepeatedCallsReturnSameInstance)
{
	const SharedVector* const first = &SharedVector::instance();
	const SharedVector* const second = &SharedVector::instance();

	EXPECT_EQ(first, second);
}

TEST(SharedVectorSingleton, InstanceIsSharedAcrossThreads)
{
	std::atomic<bool> go{false};
	std::vector<const SharedVector*> seen(thread_count, nullptr);
	std::vector<std::thread> threads;

	for (std::size_t i = 0; i < thread_count; ++i) {
		threads.emplace_back([&go, &seen, i] {
			wait_for(go);
			seen[i] = &SharedVector::instance();
		});
	}
	go.store(true, std::memory_order_release);
	for (std::thread& t : threads) {
		t.join();
	}

	const SharedVector* const expected = &SharedVector::instance();
	for (const SharedVector* const p : seen) {
		EXPECT_EQ(p, expected);
	}
}

TEST(SharedVectorSingleton, StateIsSharedBetweenReferences)
{
	SharedVector& first = SharedVector::instance();
	const SharedVector& second = SharedVector::instance();

	first.update(a_x, a_y, a_z);

	EXPECT_TRUE(has_magnitude(second.get_snapshot(), a_magnitude));
}

TEST(SharedVectorSingleton, CannotBeCopiedOrMoved)
{
	EXPECT_FALSE(std::is_copy_constructible_v<SharedVector>);
	EXPECT_FALSE(std::is_copy_assignable_v<SharedVector>);
	EXPECT_FALSE(std::is_move_constructible_v<SharedVector>);
	EXPECT_FALSE(std::is_move_assignable_v<SharedVector>);
}

TEST(SharedVectorSingleton, CannotBeConstructedOrDestroyedByClients)
{
	EXPECT_FALSE(std::is_default_constructible_v<SharedVector>);
	EXPECT_FALSE(std::is_destructible_v<SharedVector>);
}

// ---------------------------------------------------------------------------
// Single-threaded behavior
// ---------------------------------------------------------------------------

TEST(SharedVectorBehavior, SnapshotReflectsUpdate)
{
	SharedVector& sv = SharedVector::instance();

	sv.update(a_x, a_y, a_z);

	const common::Vector3D snapshot = sv.get_snapshot();
	EXPECT_TRUE(has_magnitude(snapshot, a_magnitude));
	EXPECT_FALSE(has_magnitude(snapshot, b_magnitude));
}

TEST(SharedVectorBehavior, LastUpdateWins)
{
	SharedVector& sv = SharedVector::instance();

	sv.update(a_x, a_y, a_z);
	sv.update(b_x, b_y, b_z);

	const common::Vector3D snapshot = sv.get_snapshot();
	EXPECT_TRUE(has_magnitude(snapshot, b_magnitude));
	EXPECT_FALSE(has_magnitude(snapshot, a_magnitude));
}

TEST(SharedVectorBehavior, SnapshotIsAnIndependentCopy)
{
	SharedVector& sv = SharedVector::instance();
	sv.update(a_x, a_y, a_z);

	const common::Vector3D before = sv.get_snapshot();
	sv.update(b_x, b_y, b_z);

	EXPECT_TRUE(has_magnitude(before, a_magnitude));
	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), b_magnitude));
}

TEST(SharedVectorBehavior, RepeatedSnapshotsAreEqual)
{
	SharedVector& sv = SharedVector::instance();
	sv.update(b_x, b_y, b_z);

	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), b_magnitude));
	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), b_magnitude));
}

TEST(SharedVectorBehavior, SnapshotIsCallableThroughConstReference)
{
	SharedVector::instance().update(a_x, a_y, a_z);
	const SharedVector& csv = SharedVector::instance();

	EXPECT_TRUE(has_magnitude(csv.get_snapshot(), a_magnitude));
}

TEST(SharedVectorBehavior, ZeroVector)
{
	SharedVector& sv = SharedVector::instance();

	sv.update(0., 0., 0.);

	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), 0.));
}

TEST(SharedVectorBehavior, BoundaryValues)
{
	SharedVector& sv = SharedVector::instance();

	sv.update(100., 0., 0.);
	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), 100.));

	sv.update(0., 100., 0.);
	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), 100.));

	sv.update(0., 0., 100.);
	EXPECT_TRUE(has_magnitude(sv.get_snapshot(), 100.));
}

TEST(SharedVectorBehavior, RangeCheckOnSnapshot)
{
	SharedVector& sv = SharedVector::instance();
	sv.update(a_x, a_y, a_z);

	const common::Vector3D snapshot = sv.get_snapshot();

	EXPECT_TRUE(snapshot.is_in_range(0., 10.));
	EXPECT_TRUE(snapshot.is_in_range(5., 10.));
	EXPECT_TRUE(snapshot.is_in_range(0., 5.));
	EXPECT_FALSE(snapshot.is_in_range(5.1, 10.));
	EXPECT_FALSE(snapshot.is_in_range(0., 4.9));
}

// ---------------------------------------------------------------------------
// Thread safety
// ---------------------------------------------------------------------------

// Writers alternate between two vectors while readers take snapshots. A
// snapshot must always be exactly one of the two (no torn reads).
TEST(SharedVectorThreadSafety, ReadersNeverSeeTornValues)
{
	SharedVector::instance().update(a_x, a_y, a_z);

	std::atomic<bool> go{false};
	std::atomic<std::size_t> writers_left{thread_count};
	std::atomic<std::size_t> torn{0};
	std::atomic<std::size_t> reads{0};
	std::vector<std::thread> threads;

	for (std::size_t w = 0; w < thread_count; ++w) {
		threads.emplace_back([&, w] {
			wait_for(go);
			for (std::size_t i = 0; i < iterations; ++i) {
				if (((i + w) % 2) == 0) {
					SharedVector::instance().update(a_x, a_y, a_z);
				} else {
					SharedVector::instance().update(b_x, b_y, b_z);
				}
			}
			writers_left.fetch_sub(1, std::memory_order_acq_rel);
		});
	}
	for (std::size_t r = 0; r < thread_count; ++r) {
		threads.emplace_back([&] {
			wait_for(go);
			do {
				const common::Vector3D s = SharedVector::instance().get_snapshot();
				if (!has_magnitude(s, a_magnitude) && !has_magnitude(s, b_magnitude)) {
					torn.fetch_add(1, std::memory_order_relaxed);
				}
				reads.fetch_add(1, std::memory_order_relaxed);
			} while (writers_left.load(std::memory_order_acquire) != 0);
		});
	}

	go.store(true, std::memory_order_release);
	for (std::thread& t : threads) {
		t.join();
	}

	EXPECT_EQ(torn.load(), 0U);
	EXPECT_GT(reads.load(), 0U);
}

// Every writer writes its own value; afterwards the state must be exactly
// one of the written values.
TEST(SharedVectorThreadSafety, ConcurrentWritersLeaveAWrittenValue)
{
	std::atomic<bool> go{false};
	std::vector<std::thread> threads;

	for (std::size_t w = 0; w < thread_count; ++w) {
		threads.emplace_back([&, w] {
			wait_for(go);
			for (std::size_t i = 0; i < iterations; ++i) {
				if ((w % 2) == 0) {
					SharedVector::instance().update(a_x, a_y, a_z);
				} else {
					SharedVector::instance().update(b_x, b_y, b_z);
				}
			}
		});
	}
	go.store(true, std::memory_order_release);
	for (std::thread& t : threads) {
		t.join();
	}

	const common::Vector3D s = SharedVector::instance().get_snapshot();
	EXPECT_TRUE(has_magnitude(s, a_magnitude) || has_magnitude(s, b_magnitude));
}

// With no writers, concurrent readers all observe the same value.
TEST(SharedVectorThreadSafety, ConcurrentReadersObserveSameValue)
{
	SharedVector::instance().update(b_x, b_y, b_z);

	std::atomic<bool> go{false};
	std::atomic<std::size_t> mismatches{0};
	std::vector<std::thread> threads;

	for (std::size_t r = 0; r < thread_count; ++r) {
		threads.emplace_back([&] {
			wait_for(go);
			for (std::size_t i = 0; i < iterations; ++i) {
				if (!has_magnitude(
							SharedVector::instance().get_snapshot(), b_magnitude)) {
					mismatches.fetch_add(1, std::memory_order_relaxed);
				}
			}
		});
	}
	go.store(true, std::memory_order_release);
	for (std::thread& t : threads) {
		t.join();
	}

	EXPECT_EQ(mismatches.load(), 0U);
}

// A writer's update is visible to a reader that is told (via an atomic
// flag) that the update has completed.
TEST(SharedVectorThreadSafety, UpdateIsVisibleAfterSynchronization)
{
	SharedVector::instance().update(a_x, a_y, a_z);

	std::atomic<bool> updated{false};
	std::atomic<bool> visible{false};

	std::thread reader([&] {
		while (!updated.load(std::memory_order_acquire)) {
			std::this_thread::yield();
		}
		visible.store(
			has_magnitude(SharedVector::instance().get_snapshot(), b_magnitude));
	});
	std::thread writer([&] {
		SharedVector::instance().update(b_x, b_y, b_z);
		updated.store(true, std::memory_order_release);
	});

	writer.join();
	reader.join();

	EXPECT_TRUE(visible.load());
}
