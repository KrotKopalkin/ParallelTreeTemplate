#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <ctime>
#include <set>
#include <ParallelTree.hpp>

using namespace std;

int f(vector<vector<int>> x, vector<int> p, int b) { // функция минимизации
    for (int i(0); i < x[0].size(); ++i) {
        int min = INT_MAX;
        for (auto j : p) {
            if (x[j][i] < min) {
                min = x[j][i];
            }
        }
        b -= min;
    }
    //cout << "b = " << b << endl;
    if (b >= 0) return p.size();
    return INT_MAX;
}

class ExampleRecord : public Record
{
public:
    vector<vector<int>> x;
    vector<int> p;
    int b;

    ExampleRecord(vector<vector<int>> x, vector<int> p, int b) : x(x), p(p), b(b) {}

    bool betterThan(const Record& other) const override {
        const ExampleRecord& otherCast = static_cast<const ExampleRecord&>(other);
        return f(x, p, b) < f(otherCast.x, otherCast.p, otherCast.b);
    }

    std::unique_ptr<Record> clone() const override {
        return std::make_unique<ExampleRecord>(*this);
    }
};

class ExampleNode : public Node {
public:
    vector<vector<int>> x;
    vector<int> p;
    vector<int> N;
    int k;
    int I;
    int b;
    int node;

    ExampleNode(vector<vector<int>> x, vector<int> N, int k, int b) : x(x), N(N), k(k), I(0), b(b) {}

    std::vector< std::unique_ptr<Node> > process(Record& record) override
    {

        ExampleRecord& recordCast = static_cast<ExampleRecord&>(record);

        std::vector< std::unique_ptr<Node> > childNodes;

        if (I == k) {
            if (f(x, p, b) < f(recordCast.x, recordCast.p, recordCast.b))
                recordCast.p = p;
            return childNodes;
        }
        else {
            I += 1;
            vector<int> temp = N;
            for (int i(0); i < N.size() - k + I; ++i) {
                temp = N;
                node = N[i];
                p.push_back(node);
                /*for (int i(0); i < p.size(); ++i) {
                  cout << p[i] << " ";
                }
                cout << endl;*/
                if (f(x, p, b) < f(recordCast.x, recordCast.p, recordCast.b))
                    recordCast.p = p;
                temp.erase(temp.begin() + i);
                swap(temp, N);
                childNodes.emplace_back(new ExampleNode(*this));
                swap(temp, N);
                p.pop_back();
            }
            return childNodes;
        }
    }

    bool hasHigherPriority(const Node& other) const override {
        const ExampleNode& otherCast = static_cast<const ExampleNode&>(other);
        return f(x, p, b) < f(otherCast.x, otherCast.p, otherCast.b);
    }
};

int main()
{

    vector<vector<int>> matrix = { {1,1,1,1,3,1,1,1,1},
                                   {1,1,1,1,1,1,1,1,3},
                                   {9,3,4,5,9,6,4,9,9},
                                   {9,3,4,5,9,6,4,9,9},
                                   {9,8,7,6,9,4,3,9,6} };

    int k = matrix.size() - 1;
    int b = 20;
    vector<int> N;
    for (int i(0); i < matrix.size(); ++i) N.push_back(i);
    vector<int> resized_for_record = N;
    resized_for_record.resize(k + 1);
    if (f(matrix, resized_for_record, b) == INT_MAX) {
        cout << "There is no solution here";
        return 0;
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    ExampleRecord initialRecord(matrix, resized_for_record, b);
    unique_ptr<ExampleNode> root = make_unique<ExampleNode>(matrix, N, k, b);
    unique_ptr<Record> bestSolution = parallelTree(move(root), initialRecord, 1);
    const ExampleRecord* bestSolutionCast = reinterpret_cast<const ExampleRecord*>(bestSolution.get());
    auto t2 = std::chrono::high_resolution_clock::now();
    auto tt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    cout << endl << "Runtime : " << tt << endl;
    for (int i(0); i < bestSolutionCast->p.size(); ++i) {
        cout << bestSolutionCast->p[i] + 1 << " ";
    }

    return 0;
}
