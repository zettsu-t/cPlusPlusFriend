#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <tuple>
#include <time.h>
#include <gtest/gtest.h>
#include "cppFriends.hpp"

// TLSを使わなくても、単にスレッド起動時の引数で渡せば済む
class TestThreads : public ::testing::Test {};

TEST_F(TestThreads, All) {
    struct ThreadEnv {
        int value;
    };

    ThreadEnv env1 {1};
    ThreadEnv env2 {2};
    {
        std::thread thr1([&env1](void) { std::cout << env1.value; });
        std::thread thr2([&env2](void) { std::cout << env2.value; });
        thr1.join();
        thr2.join();
    }

    std::cout << "\n";
}

// 排他制御が完全でないと何が起きるか確認する実験だが、
// memory fenceがないと競合するケースが再現できていない

class TestMemoryFence : public ::testing::Test {
protected:
    using DataElement = int;
    struct Data {
        // std::atomicを使うときは明示的に初期化する。自動的に0になる訳ではない。
        std::atomic<DataElement> element {0};
        // キャッシュのラインサイズ分だけ他のatomic変数と離す。
        // C++17ではこのサイズ(64)を取得できる。
        uint8_t gap[64];
    };
    using DataSet = std::vector<Data>;
    using Count = DataElement;
    static constexpr DataElement WriteLocked = 1;
    static constexpr Count TrialCountSpin = 2000000;
    static constexpr Count TrialCountMutex = 200000; // Mutexは時間が掛かり過ぎ

    enum class MODE {
        ASYNC,  // 同期処理を何もしないのでメチャクチャになる(というのを作りたい)
        SPIN,   // Compare and Swapによるスピンロックで同期する
        MUTEX,  // std::mutexで同期する
    };

    // データを更新する
    class Producer {
    public:
        Producer(MODE mode, Count loopCount, DataSet& dataSet,
                 std::mutex& mx, Data& ready, Data& reader, Data& writer) :
            mode_(mode), loopCount_(loopCount), dataSet_(dataSet),
            mutex_(mx), ready_(ready), reader_(reader), writer_(writer) {
        }

        virtual ~Producer(void) = default;

        void Run(void) {
            // データを消費するスレッドが起きるまで少し待つ
            timespec req {0, 10000000};
            timespec rem = req;
            for(;;) {
                auto result = nanosleep(&req, &rem);
                if (!result) {
                    break;
                }

                if (result == EINTR) {
                    // Spurious wakeup
                    req = rem;
                } else {
                    break;
                }
            }

            ready_.element = 1;
            switch(mode_) {
            case MODE::ASYNC:
                runIncorrectly();
                break;
            case MODE::SPIN:
                runInSpinning();
                break;
            case MODE::MUTEX:
                runInMutex();
                break;
            default:
                break;
            }
        }

    private:
        MODE  mode_ {MODE::ASYNC};
        Count loopCount_ {0};
        DataSet& dataSet_;
        std::mutex& mutex_;
        Data& ready_;
        Data& reader_;
        Data& writer_;

        void runIncorrectly(void) {
            // これを作る
        }

        void runInSpinning(void) {
            for(Count i = 0; i<loopCount_; ++i) {
                // Writeに対する排他を確保する
                while(writer_.element.exchange(WriteLocked, std::memory_order_seq_cst) == WriteLocked) {}

                // すべてのatomic要素を更新する
                for(auto& d : dataSet_) {
                    ++d.element;
                }
                // 排他制御を出る前に、他スレッドから見て要素が更新されているようにする
                // C++11標準。std::atomic以外の変数の順序性は指示しない。
                std::atomic_thread_fence(std::memory_order_seq_cst);
#if 0
                // x86なら、std::atomic以外の変数の順序性も巻き込んで指示する
                __sync_synchronize();
                asm volatile (
                    "mfence \n\t"
                    :::);
#endif

                // Writeに対する排他を解放する
                writer_.element = 0;
            }
        }

        void runInMutex(void) {
            for(Count i = 0; i<loopCount_; ++i) {
                // ブロックスコープ内を排他制御する。mfenceも行う。
                std::lock_guard<std::mutex> lock{mutex_};

                for(auto& d : dataSet_) {
                    ++d.element;
                }
            }
        }
    };

    class Consumer {
    public:
        Consumer(MODE mode, Count loopCount, DataSet& dataSet,
                 std::mutex& mx, Data& ready, Data& reader, Data& writer) :
            mode_(mode), loopCount_(loopCount), dataSet_(dataSet),
            mutex_(mx), ready_(ready), reader_(reader), writer_(writer) {
        }
        virtual ~Consumer(void) = default;

        void Run(void) {
            while(!ready_.element) {}

            switch(mode_) {
            case MODE::ASYNC:
                runIncorrectly();
                break;
            case MODE::SPIN:
                runInSpinning();
                break;
            case MODE::MUTEX:
                runInMutex();
                break;
            default:
                break;
            }
        }

        Count GetCount(void) const {
            return count_;
        }

        void runIncorrectly(void) {
            // これを作る
        }

        void runInSpinning(void) {
            for(Count i = 0; i<loopCount_; ++i) {
                // Writeに対する排他を確保する
                // 同時に複数のreadersからは読めない
                // (工夫すればできそうだが、readerが多すぎるとライブロックしてwriterが更新できなくなる)
                while(writer_.element.exchange(WriteLocked, std::memory_order_seq_cst) == WriteLocked) {}

                // happens beforeは受け側でも指示する
                std::atomic_thread_fence(std::memory_order_seq_cst);
                checkConsistency();

                // readerは読むだけなのでfenceは必要ない
                writer_.element = 0;
            }
        }

        void runInMutex(void) {
            for(Count i = 0; i<loopCount_; ++i) {
                // ブロックスコープ内を排他制御する
                std::lock_guard<std::mutex> lock{mutex_};
                checkConsistency();
            }
        }

        void checkConsistency(void) {
            DataElement prev = std::numeric_limits<decltype(prev)>::max();
            for(auto& d : dataSet_) {
                DataElement e = d.element;
                // 更新順序が逆転している
                // 周回をまたぐことはないのでこれでよい
                if (prev < e) {
                    ++count_;
                    break;
                }
                prev = e;
            }
        }

    private:
        MODE  mode_ {MODE::ASYNC};
        Count loopCount_ {0};
        DataSet& dataSet_;
        std::mutex& mutex_;
        Data& ready_;
        Data& reader_;
        Data& writer_;
        Count count_ {0};
    };

    void exec(MODE mode, Count loopCount, Count& minCount, Count& maxCount) {
        minCount = 0;
        maxCount = 0;

        DataSet  dataSet(16);  // キャッシュの連想度を超えるくらい
        std::mutex mx;
        Data ready  {{0},{0}};
        Data reader {{0},{0}};
        Data writer {{0},{0}};
        constexpr Count readerSize = 3;  // writerと併せて論理コアくらい

        // One-writer, multi-reader
        Producer producer(mode, loopCount, dataSet, mx, ready, reader, writer);
        std::vector<std::unique_ptr<Consumer>> consumers;
        for(Count i=0; i<readerSize; ++i) {
            consumers.push_back(std::make_unique<Consumer>(mode, loopCount, dataSet, mx, ready, reader, writer));
        }

        {
            // 並行処理を作って、後で実行できるようにする
            std::vector<std::future<void>> futureSet;
            for(auto& p : consumers) {
                // futureの方が寿命が短いのだから、shared_ptrにする必要はない
                Consumer* pConsumer = p.get();
                futureSet.push_back(std::async(std::launch::async, [=](void) -> void { pConsumer->Run(); }));
            }
            futureSet.push_back(std::async(std::launch::async, [&](void) -> void { producer.Run(); }));

            // 並行して評価して、結果がそろうのを待つ
            for(auto& f : futureSet) {
                f.get();
            }
        }

        for(auto& d : dataSet) {
            EXPECT_EQ(loopCount, d.element);
        }

        decltype(consumers)::iterator minComsumer;
        decltype(consumers)::iterator maxComsumer;
        std::tie(minComsumer, maxComsumer) = std::minmax_element(
            consumers.begin(), consumers.end(),
            [] (const decltype(consumers)::value_type& l, const decltype(consumers)::value_type& r)
            { return l->GetCount() < r->GetCount(); });

        minCount = (*minComsumer)->GetCount();
        maxCount = (*maxComsumer)->GetCount();
        return;
    }
};

TEST_F(TestMemoryFence, Async) {
    // これを作る
    return;
}

TEST_F(TestMemoryFence, Spin) {
    if (std::thread::hardware_concurrency() <= 1) {
        return;
    }

    Count minCount = 0;
    Count maxCount = 0;
    exec(MODE::SPIN, TrialCountSpin, minCount, maxCount);

    std::cout << "consumer.GetCount(spin) = count " << minCount << "\n";
    EXPECT_FALSE(minCount);
    EXPECT_FALSE(maxCount);
    return;
}

TEST_F(TestMemoryFence, Mutex) {
    if (std::thread::hardware_concurrency() <= 1) {
        return;
    }

    Count minCount = 0;
    Count maxCount = 0;
    // あまりにも時間が掛かるので、テストサイズを減らす
    exec(MODE::MUTEX, TrialCountMutex, minCount, maxCount);

    std::cout << "consumer.GetCount(mutex) = count " << minCount << "\n";
    EXPECT_FALSE(minCount);
    EXPECT_FALSE(maxCount);
    return;
}

namespace {
    // 他のスレッドが起きるまで少し待つ
    void Delay(bool valid) {
        if (valid) {
            std::chrono::milliseconds dur(10);
            std::this_thread::sleep_for(dur);
        }
    }
}

// Condition variableを使って、スレッド間でハンドシェイクする
class TestConditionVariable : public ::testing::Test {
protected:
    using SharedValue = int;
    using SharedAtomic = std::atomic<SharedValue>;

    // データを更新する
    class Sender {
    public:
        Sender(SharedValue count, bool delayed, SharedAtomic& value, SharedAtomic& received,
               std::mutex& mx, std::condition_variable& cvSent, std::condition_variable& cvReceived) :
            count_(count), delayed_(delayed), value_(value), received_(received),
            mutex_(mx), cvSent_(cvSent), cvReceived_(cvReceived) {
            return;
        }

        virtual ~Sender(void) = default;

        SharedValue Exec(void) {
            // 指定されていれば、スレッドの開始を遅らせる
            Delay(delayed_);
            SharedValue count = 0;

            for(SharedValue i=0; i<count_; ++i) {
                std::unique_lock<std::mutex> lock(mutex_);
                // ループの初回は受信済になっている
                cvReceived_.wait(lock, [&](void) -> bool { return received_.load(); });
                received_ = 0;

                // 共有データを更新する
                value_ += 1;
                ++count;
                // 更新したことを通知する
                cvSent_.notify_all();
            }

            return count;
        }

    private:
        SharedValue count_ {0};
        bool delayed_ {false};
        SharedAtomic& value_;
        SharedAtomic& received_;
        std::mutex& mutex_;
        std::condition_variable& cvSent_;
        std::condition_variable& cvReceived_;
    };

    // データを更新する
    class Receiver {
    public:
        Receiver(SharedValue count, bool delayed, SharedAtomic& value, SharedAtomic& received,
               std::mutex& mx, std::condition_variable& cvSent, std::condition_variable& cvReceived) :
            count_(count), delayed_(delayed), value_(value), received_(received),
            mutex_(mx), cvSent_(cvSent), cvReceived_(cvReceived) {
            // 初期状態は受信済にする
            received_ = 1;
            previousValue_ = value_.load();
            return;
        }

        virtual ~Receiver(void) = default;

        SharedValue Exec(void) {
            // 指定されていれば、スレッドの開始を遅らせる
            Delay(delayed_);
            SharedValue count = 0;

            // カウンタいっぱいまで行ったら、送信完了とする
            for(SharedValue i=0; i<count_; ++i) {
                std::unique_lock<std::mutex> lock(mutex_);
                // すでに更新されていたら待たない
                // 更新しなければ、更新したことを通知されるまで待つ
                // spurious wakeupでは起きない
                cvSent_.wait(lock, [&](void) -> bool { return (value_.load() > previousValue_); });

                // 共有データを受け取る
                previousValue_ = value_.load();
                ++count;
                received_ = 1;
                // 受信したことを通知する
                cvReceived_.notify_all();
            }

            return count;
        }

    private:
        SharedValue count_ {0};
        bool delayed_ {false};
        SharedAtomic& value_;
        SharedAtomic& received_;
        std::mutex& mutex_;
        std::condition_variable& cvSent_;
        std::condition_variable& cvReceived_;
        SharedValue previousValue_ {0};
    };

    // テストを実行する
    void exec(SharedValue count, bool senderDelayed, bool receiverDelayed) {
        SharedAtomic value {0};     // 初期値を明示的に与える必要がある
        SharedAtomic received {1};  // 初期状態は受信済
        std::mutex mx;
        std::condition_variable cvSent;
        std::condition_variable cvReceived;
        Sender sender(count, senderDelayed, value, received, mx, cvSent, cvReceived);
        Receiver receiver(count, receiverDelayed, value, received, mx, cvSent, cvReceived);

        std::future<SharedValue> futureSender = std::async(std::launch::async, [&](void) -> auto { return sender.Exec(); });
        std::future<SharedValue> futureReceiver = std::async(std::launch::async, [&](void) -> auto { return receiver.Exec(); });
        auto actualSender = futureSender.get();
        auto actualReceiver = futureReceiver.get();
        EXPECT_EQ(count, value.load());
        EXPECT_EQ(count, actualSender);
        EXPECT_EQ(count, actualReceiver);
    }
};

TEST_F(TestConditionVariable, Short) {
    // スレッドを起動-終了して、Delayで待つので(これが長い)、Cygwinだとループ30回で1秒少々掛かる
    for(int i=0; i<30; ++i) {
        exec(10, true, false);
        exec(10, false, true);
    }
}

TEST_F(TestConditionVariable, Long) {
    for(int i=0; i<4; ++i) {
        // カウント10000回、以下一組、1ループ当たり、Cygwinでは0.5秒少々掛かる
        exec(10000, false, false);
        exec(10000, true, false);
        exec(10000, true, true);
    }
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
