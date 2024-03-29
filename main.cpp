#include <iostream>
#include <utility>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>


using namespace std;

typedef pair<int, int> ii;

const int SEED = 42;

/*
This function generates all the intervals for merge sort iteratively, given the
range of indices to sort. Algorithm runs in O(n).

Parameters:
start : int - start of range
end : int - end of range (inclusive)

Returns a list of integer pairs indicating the ranges for merge sort.
*/
vector<ii> generate_intervals(int start, int end);

/*
This function performs the merge operation of merge sort.

Parameters:
array : vector<int> - array to sort
s     : int         - start index of merge
e     : int         - end index (inclusive) of merge
*/
void merge(vector<int> &array, int s, int e);
void concurrent_merge_sort(vector<int> &array, const vector<ii> &intervals, int threadCount);
void concurrent_merge_sort_worker(vector<int> &array, const vector<ii> &intervals);

int main()
{
    // TODO: Seed your randomizer
    mt19937 rng(SEED);

    // TODO: Get array size and thread count from user
    int N, threadCount;
    cout << "Enter array size: ";
    cin >> N;
    cout << "Enter thread count: ";
    cin >> threadCount;

    // TODO: Generate a random array of given size
    vector<int> array(N);
    for (int i = 0; i < N; ++i)
    {
        array[i] = i + 1;
    }
    shuffle(array.begin(), array.end(), rng);

    // TODO: Call the generate_intervals method to generate the merge sequence
    vector<ii> intervals = generate_intervals(0, N - 1);

    // TODO: Call merge on each interval in sequence
    // start timer
    auto start = chrono::high_resolution_clock::now();

    if (threadCount > 1)
    {
        concurrent_merge_sort(array, intervals, threadCount);
    }
    else
    {
        for (const auto &interval : intervals)
        {
            merge(array, interval.first, interval.second);
        }
    }
    
    // end timer
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Time taken: " << elapsed.count() << " seconds\n";

    // sanity check if the array is sorted
    if (is_sorted(array.begin(), array.end()))
    {
        cout << "Array is correctly sorted.\n";
    }
    else
    {
        cout << "Array is not sorted.\n";
    }

    return 0;
}

vector<ii> generate_intervals(int start, int end)
{
    vector<ii> frontier;
    frontier.push_back(ii(start, end));
    int i = 0;
    while (i < (int)frontier.size())
    {
        int s = frontier[i].first;
        int e = frontier[i].second;

        i++;

        // if base case
        if (s == e)
        {
            continue;
        }

        // compute midpoint
        int m = s + (e - s) / 2;

        // add prerequisite intervals
        frontier.push_back(ii(m + 1, e));
        frontier.push_back(ii(s, m));
    }

    vector<ii> retval;
    for (int i = (int)frontier.size() - 1; i >= 0; i--)
    {
        retval.push_back(frontier[i]);
    }
    return retval;
}

void merge(vector<int> &array, int s, int e)
{
    int m = s + (e - s) / 2;
    vector<int> left;
    vector<int> right;
    for (int i = s; i <= e; i++)
    {
        if (i <= m)
        {
            left.push_back(array[i]);
        }
        else
        {
            right.push_back(array[i]);
        }
    }
    int l_ptr = 0, r_ptr = 0;

    for (int i = s; i <= e; i++)
    {
        if (l_ptr < (int)left.size() && (r_ptr == (int)right.size() || left[l_ptr] <= right[r_ptr]))
        {
            array[i] = left[l_ptr];
            l_ptr++;
        }
        else if (r_ptr < (int)right.size())
        {
            array[i] = right[r_ptr];
            r_ptr++;
        }
    }
}

void concurrent_merge_sort_worker(vector<int> &array, const vector<ii> &intervals) {
    for (const auto &interval : intervals) {
        merge(array, interval.first, interval.second);
    }
}

void concurrent_merge_sort(vector<int> &array, const vector<ii> &intervals, int threadCount) {
    int N = array.size();
    vector<thread> threads;

    int intervalsPerThread = intervals.size() / threadCount;

    // Create threads and distribute intervals among them
    for (int i = 0; i < threadCount; ++i) {
        int startIdx = i * intervalsPerThread;
        int endIdx = (i == threadCount - 1) ? intervals.size() - 1 : startIdx + intervalsPerThread - 1;

        vector<ii> threadIntervals(intervals.begin() + startIdx, intervals.begin() + endIdx + 1);

        threads.emplace_back(concurrent_merge_sort_worker, ref(array), threadIntervals);
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Phase 2: Sequentially merge the sorted chunks
    for (int size = 1; size < N; size *= 2) {
        for (int leftStart = 0; leftStart < N; leftStart += 2 * size) {
            int mid = leftStart + size - 1;
            int rightEnd = min(leftStart + 2 * size - 1, N - 1);
            if (mid < rightEnd) { // Check if there is anything to merge
                merge(array, leftStart, rightEnd);
            }
        }
    }
}