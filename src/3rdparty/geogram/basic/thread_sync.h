/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef __GEOGRAM_BASIC_THREAD_SYNC__
#define __GEOGRAM_BASIC_THREAD_SYNC__

#include <geogram/basic/common.h>
#include <geogram/basic/atomics.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/argused.h>
#include <vector>

#ifdef GEO_OS_APPLE
#  define GEO_USE_DEFAULT_SPINLOCK_ARRAY
#endif

/**
 * \file geogram/basic/thread_sync.h
 * \brief Functions and classes for process manipulation
 */

namespace GEO {

    namespace Process {

#if defined(GEO_OS_ANDROID)

        /** A lightweight synchronization structure. */
        typedef arm_mutex_t spinlock;

        /**
         * \brief Loops until \p x is available then reserves it.
         * \param[in] x a spinlock that should be available.
         */
        inline void acquire_spinlock(spinlock& x) {
            lock_mutex_arm(&x);
        }

        /**
         * \brief Makes \p x available to other threads.
         * \param[in] x a spinlock that should be reserved.
         */
        inline void release_spinlock(spinlock& x) {
            unlock_mutex_arm(&x);
        }

#elif defined(GEO_OS_LINUX)

        /** A lightweight synchronization structure. */
        typedef unsigned char spinlock;

        /**
         * \brief Loops until \p x is available then reserve it.
         * \param[in] x a spinlock that should be available.
         */
        inline void acquire_spinlock(volatile spinlock& x) {
            while(__sync_lock_test_and_set(&x, 1) == 1) {
                // Intel recommends to have a PAUSE asm instruction
                // in the spinlock loop.
                geo_pause();
            }
        }

        /**
         * \brief Makes \p x available to other threads.
         * \param[in] x a spinlock that should be reserved.
         */
        inline void release_spinlock(volatile spinlock& x) {
            // Note: Since on intel processor, memory writes
            // (of data types <= bus size) are atomic, we could
            // simply write 'x=0' instead, but this would
            // lack the 'memory barrier with release semantics'
            // required to avoid compiler and/or processor
            // reordering (important for relaxed memory
            // models such as Itanium processors).
            __sync_lock_release(&x);
        }

#elif defined(GEO_OS_APPLE)

        /** A lightweight synchronization structure. */
        typedef OSSpinLock spinlock;

        inline void acquire_spinlock(volatile spinlock& x) {
            OSSpinLockLock(&x);
        }

        inline void release_spinlock(volatile spinlock& x) {
            OSSpinLockUnlock(&x);
        }

#elif defined(GEO_OS_WINDOWS)

        /** A lightweight synchronization structure. */
        typedef short spinlock;

        inline void acquire_spinlock(volatile spinlock& x) {
            while(_InterlockedCompareExchange16(&x, 1, 0) == 1) {
                // Intel recommends to have a PAUSE asm instruction
                // in the spinlock loop. Under MSVC/Windows,
                // YieldProcessor() is a macro that calls the
                // (undocumented) _mm_pause() intrinsic function
                // that generates a PAUSE opcode.
                YieldProcessor();
            }
            // We do not need _ReadBarrier() here since
            // _InterlockedCompareExchange16
            // "acts as a full barrier in VC2005" according to the doc
        }

        inline void release_spinlock(volatile spinlock& x) {
            _WriteBarrier();   // prevents compiler reordering
            x = 0;
        }

#endif

#if defined(GEO_USE_DEFAULT_SPINLOCK_ARRAY)

        // TODO: implement memory-efficient version for
        // MacOSX MacOSX does have atomic bit
        // manipulation routines (OSAtomicTestAndSet()),
        // and also  has OSAtomicAnd32OrigBarrier() and
        // OSAtomicOr32OrigBarrier() functions that can
        // be used instead (Orig for 'return previous value'
        // and Barrier for ('include a memory barrier').
        // Android needs additional routines in atomics.h/atomics.cpp

        /**
         * \brief An array of light-weight synchronisation
         *  primitives (spinlocks).
         *
         * \details This is the reference implementation, that uses
         * a std::vector of spinlock. There are more efficient
         * implementations for Linux and Windows.
         *
         * \see acquire_spinlock(), release_spinlock()
         */
        class SpinLockArray {
        public:
            /**
             * \brief Constructs a new SpinLockArray of size 0.
             */
            SpinLockArray() {
            }

            /**
             * \brief Constructs a new SpinLockArray of size \p size_in.
             * \param[in] size_in number of spinlocks in the array.
             */
            SpinLockArray(index_t size_in) {
                resize(size_in);
            }

            /**
             * \brief Resizes a SpinLockArray.
             * \details All the spinlocks are reset to 0.
             * \param[in] size_in The desired new size.
             */
            void resize(index_t size_in) {
                spinlocks_.assign(size_in, 0);
            }

            /**
             * \brief Resets size to 0 and clears all the memory.
             */
            void clear() {
                spinlocks_.clear();
            }

            /**
             * \brief Gets the number of spinlocks in this array.
             */
            index_t size() const {
                return index_t(spinlocks_.size());
            }

            /**
             * \brief Acquires a spinlock at a given index
             * \details Loops until spinlock at index \p i is available then
             * reserve it.
             * \param[in] i index of the spinlock
             */
            void acquire_spinlock(index_t i) {
                geo_debug_assert(i < size());
                GEO::Process::acquire_spinlock(spinlocks_[i]);
            }

            /**
             * \brief Releases a spinlock at a given index
             * \details Makes spinlock at index \p i availabe to other threads.
             * \param[in] i index of the spinlock
             */
            void release_spinlock(index_t i) {
                geo_debug_assert(i < size());
                GEO::Process::release_spinlock(spinlocks_[i]);
            }

        private:
            std::vector<spinlock> spinlocks_;
        };

#elif defined(GEO_OS_ANDROID)

        /**
         * \brief An array of light-weight synchronisation
         *  primitives (spinlocks).
         *
         * \details In this implementation, storage is optimized so that
         * a single bit per spinlock is used.
         *
         */
        class SpinLockArray {
        public:
            /**
             * \brief Internal representation of SpinLockArray elements.
             * \details Each word_t represents 32 spinlocks.
             */
            typedef Numeric::uint32 word_t;

            /**
             * \brief Constructs a new SpinLockArray of size 0.
             */
            SpinLockArray() : size_(0) {
            }

            /**
             * \brief Constructs a new SpinLockArray of size \p size_in.
             * \param[in] size_in number of spinlocks in the array.
             */
            SpinLockArray(index_t size_in) : size_(0) {
                resize(size_in);
            }

            /**
             * \brief Resizes a SpinLockArray.
             * \details All the spinlocks are reset to 0.
             * \param[in] size_in The desired new size.
             */
            void resize(index_t size_in) {
                if(size_ != size_in) {
                    size_ = size_in;
                    index_t nb_words = (size_ >> 5) + 1;
                    spinlocks_.assign(nb_words, 0);
                }
            }

            /**
             * \brief Gets the number of spinlocks in this array.
             */
            index_t size() const {
                return size_;
            }

            /**
             * \brief Resets size to 0 and clears all the memory.
             */
            void clear() {
                spinlocks_.clear();
            }

            /**
             * \brief Acquires a spinlock at a given index
             * \details Loops until spinlock at index \p i is available then
             * reserve it.
             * \param[in] i index of the spinlock
             */
            void acquire_spinlock(index_t i) {
                geo_debug_assert(i < size());
                index_t w = i >> 5;
                word_t b = word_t(i & 31);
                // Loop while previously stored value has its bit set.
                while((atomic_bitset_arm(&spinlocks_[w], b)) != 0) {
                    // If somebody else has the lock, sleep.
                    //  It is important to sleep here, else atomic_bitset_arm()
                    // keeps acquiring the exclusive monitor (even for testing)
                    // and this slows down everything.
                    wait_for_event_arm();
                }
                memory_barrier_arm();
            }

            /**
             * \brief Releases a spinlock at a given index
             * \details Makes spinlock at index \p i availabe to other threads.
             * \param[in] i index of the spinlock
             */
            void release_spinlock(index_t i) {
                geo_debug_assert(i < size());
                memory_barrier_arm();
                index_t w = i >> 5;
                word_t b = word_t(i & 31);
                atomic_bitreset_arm(&spinlocks_[w], b);
                //   Now wake up the other threads that started
                // sleeping if they did not manage to acquire
                // the lock.
                send_event_arm();
            }

        private:
            std::vector<word_t> spinlocks_;
            index_t size_;
        };

#elif defined(GEO_OS_LINUX)

        /**
         * \brief An array of light-weight synchronisation
         *  primitives (spinlocks).
         *
         * \details In this implementation, storage is optimized so that
         * a single bit per spinlock is used.
         *
         * \see acquire_spinlock(), release_spinlock()
         */
        class SpinLockArray {
        public:
            /**
             * \brief Internal representation of SpinLockArray elements.
             * \details Each word_t represents 32 spinlocks.
             */
            typedef Numeric::uint32 word_t;

            /**
             * \brief Constructs a new SpinLockArray of size 0.
             */
            SpinLockArray() : size_(0) {
            }

            /**
             * \brief Constructs a new SpinLockArray of size \p size_in.
             * \param[in] size_in number of spinlocks in the array.
             */
            SpinLockArray(index_t size_in) : size_(0) {
                resize(size_in);
            }

            /**
             * \brief Resizes a SpinLockArray.
             * \details All the spinlocks are reset to 0.
             * \param[in] size_in The desired new size.
             */
            void resize(index_t size_in) {
                if(size_ != size_in) {
                    size_ = size_in;
                    index_t nb_words = (size_ >> 5) + 1;
                    spinlocks_.assign(nb_words, 0);
                }
            }

            /**
             * \brief Gets the number of spinlocks in this array.
             */
            index_t size() const {
                return size_;
            }

            /**
             * \brief Resets size to 0 and clears all the memory.
             */
            void clear() {
                spinlocks_.clear();
            }

            /**
             * \brief Acquires a spinlock at a given index
             * \details Loops until spinlock at index \p i is available then
             * reserve it.
             * \param[in] i index of the spinlock
             */
            void acquire_spinlock(index_t i) {
                geo_debug_assert(i < size());
                index_t w = i >> 5;
                index_t b = i & 31;
                while(atomic_bittestandset_x86(&spinlocks_[w], b)) {
                    // Intel recommends to have a PAUSE asm instruction
                    // in the spinlock loop. It is generated using the
                    // following intrinsic function of GCC.
                    geo_pause();
                }
            }

            /**
             * \brief Releases a spinlock at a given index
             * \details Makes spinlock at index \p i availabe to other threads.
             * \param[in] i index of the spinlock
             */
            void release_spinlock(index_t i) {
                geo_debug_assert(i < size());
                index_t w = i >> 5;
                index_t b = i & 31;
                // Note: we need here to use a synchronized bit reset
                // since &= is not atomic.
                atomic_bittestandreset_x86(&spinlocks_[w], b);
            }

        private:
            std::vector<word_t> spinlocks_;
            index_t size_;
        };

#elif defined(GEO_OS_WINDOWS)
        /**
         * \brief An array of light-weight synchronisation
         *  primitives (spinlocks).
         *
         * \details In this implementation, storage is optimized so that
         * a single bit per spinlock is used.
         *
         * \see acquire_spinlock(), release_spinlock()
         */
        class SpinLockArray {
        public:
            /**
             * \brief Internal representation of SpinLockArray elements.
             * \details Each word_t represents 32 spinlocks.
             * \internal
             * LONG is 32 bits under MSVC
             * and is what interlockedbittestand(re)set uses.
             */
            typedef LONG word_t;

            /**
             * \brief Constructs a new SpinLockArray of size 0.
             */
            SpinLockArray() : size_(0) {
            }

            /**
             * \brief Constructs a new SpinLockArray of size \p size_in.
             * \param[in] size_in number of spinlocks in the array.
             */
            SpinLockArray(index_t size_in) : size_(0) {
                resize(size_in);
            }

            /**
             * \brief Resizes a SpinLockArray.
             * \details All the spinlocks are reset to 0.
             * \param[in] size_in The desired new size.
             */
            void resize(index_t size_in) {
                if(size_ != size_in) {
                    size_ = size_in;
                    index_t nb_words = (size_ >> 5) + 1;
                    spinlocks_.assign(nb_words, 0);
                }
            }

            /**
             * \brief Gets the number of spinlocks in this array.
             */
            index_t size() const {
                return size_;
            }

            /**
             * \brief Resets size to 0 and clears all the memory.
             */
            void clear() {
                spinlocks_.clear();
            }

            /**
             * \brief Acquires a spinlock at a given index
             * \details Loops until spinlock at index \p i is available then
             * reserve it.
             * \param[in] i index of the spinlock
             */
            void acquire_spinlock(index_t i) {
                geo_debug_assert(i < size());
                index_t w = i >> 5;
                index_t b = i & 31;
                while(_interlockedbittestandset(&spinlocks_[w], b)) {
                    // Intel recommends to have a PAUSE asm instruction
                    // in the spinlock loop. Under MSVC/Windows,
                    // YieldProcessor() is a macro that calls the
                    // (undocumented) _mm_pause() intrinsic function
                    // that generates a PAUSE opcode.
                    YieldProcessor();
                }
                // We do not need here _ReadBarrier() since
                // _interlockedbittestandset
                // "acts as a full barrier in VC2005" according to the doc
            }

            /**
             * \brief Releases a spinlock at a given index
             * \details Makes spinlock at index \p i availabe to other threads.
             * \param[in] i index of the spinlock
             */
            void release_spinlock(index_t i) {
                geo_debug_assert(i < size());
                index_t w = i >> 5;
                index_t b = i & 31;
                // Note1: we need here to use a synchronized bit reset
                // since |= is not atomic.
                // Note2: We do not need here _WriteBarrier() since
                // _interlockedbittestandreset
                // "acts as a full barrier in VC2005" according to the doc
                _interlockedbittestandreset(&spinlocks_[w], b);
            }

        private:
            std::vector<word_t> spinlocks_;
            index_t size_;
        };

#else

#error Found no implementation of SpinLockArray

#endif


    }
}

#endif

