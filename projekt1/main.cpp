#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <iomanip>
#include <utility>
#include <cmath>
#include <complex>
#include <algorithm>
#include <memory>
using namespace std;

# define RECORDSIZE 24

//sorting by heat
struct Record{
    double mass;
    double specific_heat;
    double temperature_difference;
    double heat() const {
        return mass*specific_heat*temperature_difference;
    }
};

ostream& operator<<(ostream& os, Record& r) {
    os << "{M=" << r.mass
       << " c=" << r.specific_heat
       << " T=" << r.temperature_difference
       << " Q=" << r.heat() << "} ";
    return os;
}
struct HeapNode {
    Record record;
    int run_index;

    bool operator<(const HeapNode& other) const {
        return record.heat() < other.record.heat();
    }
};
class MinHeap {
private:
    vector<HeapNode> heap;
//adding new node and shifting it
    void shiftup(int index) {
        while (index>0) {
            int parentIndex=(index-1)/2;
            if (heap[index]<heap[parentIndex]) {
                swap(heap[index],heap[parentIndex]);
                index=parentIndex;
            }
            else
                break;
        }
    }
    //deleting node and shifting down
    void shiftdown(int index) {
        int size=heap.size();
        while (true) {
            int left=index*2+1;
            int right=index*2+2;
            int smallest=index;
            if (left<size && heap[left]<heap[smallest]) {
                smallest=left;
            }
            if (right<size && heap[right]<heap[smallest]) {
                smallest=right;
            }
            if (smallest!=index) {
                swap(heap[smallest],heap[index]);
            }
            else {
                break;
            }
        }
    }
public:
    bool empty() {
        return heap.empty();
    }
    HeapNode top() {
        return heap[0];
    }
    void push(HeapNode node) {
        heap.push_back(node);
        shiftup(heap.size()-1);
    }
    void pop() {
        if (heap.empty()) {
            return;
        }
//deleting smallest node
        heap[0]=heap.back();
        heap.pop_back();
        if (!heap.empty()) {
            shiftdown(0);
        }

    }
};

class DiskManager {
    int b_factor;
    int page_size;
public:
    long long read_count=0;
    long long write_count=0;
    DiskManager(const int b) {
        b_factor=b;
        page_size= RECORDSIZE*b_factor;
    }
//it returns number of records
    int readPage(fstream& file, vector<Record>& page_buffer) {
        if (page_buffer.size() != b_factor) {
            page_buffer.resize(b_factor);
        }

        file.read(reinterpret_cast<char*>(page_buffer.data()), page_size);
        streamsize bytes_read = file.gcount();
        if (bytes_read > 0) {
            read_count++;
            int records_read = bytes_read / sizeof(Record);
            for (int i = records_read; i < b_factor; ++i) {
                page_buffer[i] = Record{};
            }
            return true;
        }
        return false;
    }

    void writePage(fstream& file, const vector<Record>& page_buffer) {
        file.write(reinterpret_cast<const char*>(page_buffer.data()), page_size);
        write_count++;
    }
    int getB() const {
        return b_factor;
    }
};


//algorithm PART1
int createRuns(const string& input_filename, DiskManager&disk,int n_buffers) {
    fstream file(input_filename, ios::in| ios::binary);

    vector<Record> ram;
    vector<Record> connector;
    int b=disk.getB();
    bool loadData=true;
    int files_counter=0;
    while (loadData) {
        ram.clear();
        //1. read records from disk to buffer
        for (int i=0; i<n_buffers; ++i) {
            if (disk.readPage(file,connector)) {
                for (const auto & j : connector) {
                    ram.push_back(j);
                }
            }
            else {
                loadData=false;
                break;
            }
        }
        if (ram.empty())
            break;
        //2.Sort buffer
        sort(ram.begin(),ram.end(),[](const Record& x, const Record& y){return x.heat()<y.heat();});

        //3. write records from buffer to disk
        string output_filename = "run_" + to_string(files_counter) + ".bin";
        ++files_counter;
        fstream file_output(output_filename, ios::out| ios::binary| ios::trunc);
        connector.clear();
        for (const auto & i : ram) {
            connector.push_back(i);
            if (connector.size()==b){
                disk.writePage(file_output,connector);
                connector.clear();
            }
        }
        if (!connector.empty()) {
            while (connector.size() < b) {
                connector.push_back(Record{});
            }
            disk.writePage(file_output, connector);
        }
        file_output.close();

    }
    file.close();
    return files_counter;
}
//part2
void mergeRuns(int filesCounter,DiskManager&disk,int nofBuffers) {
    //files counter is number of runs
    int nofmergingbuffers=min(nofBuffers-1,filesCounter);
//opening input files
    string outputName;
    vector<unique_ptr<fstream>> input_files;
    for (int i=0; i<nofmergingbuffers; ++i) {
        outputName= "run_" + to_string(i) + ".bin";
        input_files.push_back( make_unique<fstream>(outputName, ios::in | ios::binary));
        Record r{};
    }
//opening output files
    fstream outputfile("output.bin",ios::out|ios::binary|ios::trunc);
    vector<vector<Record>> buffers(nofmergingbuffers);
    vector<Record> outputBuffer;
    vector<int>record_indx(nofmergingbuffers,1);
    MinHeap heap;
    int b=disk.getB();

    //initialize heap and merging buffers
    for (int i=0; i<nofmergingbuffers; ++i) {
        if (disk.readPage(*input_files[i],buffers[i])) {
            if (buffers[i][0].mass > 0) {
                heap.push({buffers[i][0],i});
                record_indx[i]=1;
            }
        }
    }

    //main merging loop
    while (!heap.empty()) {
        //smallest -> out from heap
        HeapNode node=heap.top();
        heap.pop();

        Record r= node.record;
        int run_index=node.run_index;

        //push to output buffer
        outputBuffer.push_back(r);
        if (outputBuffer.size()==b) {
            disk.writePage(outputfile,outputBuffer);
            outputBuffer.clear();
        }
    //if one of the merging buffer is fully seen we read next page from disk
        if (record_indx[run_index] >= b) {
            if (disk.readPage(*input_files[run_index], buffers[run_index])) {
                record_indx[run_index] = 0;
            } else {
                continue;
            }
        }

//new smallest record in buffer we push to heap
        if (buffers[run_index][record_indx[run_index]].mass > 0) {
            heap.push({buffers[run_index][record_indx[run_index]], run_index});
            record_indx[run_index]++;
        }

    }
    //if output buffer is not empty ( safety, it should be) we create empty records an push it

    if (!outputBuffer.empty())
    {
        while (outputBuffer.size() < b) {
            outputBuffer.push_back(Record{});
        }
        disk.writePage(outputfile, outputBuffer);
    }
    outputfile.close();
}
void generateRandomBinaryFile(const string& filename, const int n_records) {
    cout << "Generating " << n_records << " binary records to: " << filename << endl;
    fstream file(filename, ios::out | ios::binary | ios::trunc);

    mt19937 gen(random_device{}());
    uniform_real_distribution<> mass_dist(1.0, 100.0);
    uniform_real_distribution<> heat_dist(0.1, 5.0);
    uniform_real_distribution<> temp_dist(-50.0, 50.0);

    for (int i = 0; i < n_records; ++i) {
        Record r{mass_dist(gen), heat_dist(gen), temp_dist(gen)};
//cast to bin files as just bytes
        file.write(reinterpret_cast<const char*>(&r), RECORDSIZE);
    }
}
void printBinaryFile(const string& filename) {
    cout << "\n--- Reading binary file: " << filename << " ---\n";
    fstream file(filename, ios::in | ios::binary);
    Record r{};
    int i = 0;
    while (file.read(reinterpret_cast<char*>(&r), sizeof(Record))) {
        cout << "[" << i++ << "] M=" << fixed << setprecision(4) << r.mass
             << " C=" << r.specific_heat
             << " dT=" << r.temperature_difference
             << " | Q=" << r.heat() << endl;
    }
    cout << "-----------------------------------\n";
}
void printRuns(int filesCounter) {
    cout << "\n--- Reading Runs files: "  << " ---\n";
    string outputName="";
    for (int i=0; i<filesCounter; ++i) {
        outputName= "run_" + to_string(i) + ".bin";
        fstream file(outputName, ios::in | ios::binary);
        Record r{};
        int iter = 0;
        while (file.read(reinterpret_cast<char*>(&r), sizeof(Record))) {
            cout << "[runnumber: " << i << " recorditerator: "<<iter++<<" ] ""M=" << fixed << setprecision(4) << r.mass
                 << " C=" << r.specific_heat
                 << " dT=" << r.temperature_difference
                 << " | Q=" << r.heat() << endl;
        }
    }


}

int main() {
    int n;
    int nofBuffors;
    int b;
    cout << "Enter:\n1. N (number of records in the file)\n2. n (number of buffers available for sorting)\n3. b (blocking factor) \n1. N: ";
    cin>>n;
    cout<<"2.n: ";
    cin>>nofBuffors;
    cout<<"3.b: ";
    cin>>b;
    string filename = "dane.bin";
    generateRandomBinaryFile(filename,n);
    printBinaryFile(filename);

    DiskManager disk(b);
    fstream fileIn(filename, ios::in | ios::binary);
    vector<Record> myBuffer(b);

    const int filesCounter=createRuns(filename,disk,nofBuffors);
    mergeRuns(filesCounter,disk,nofBuffors);
    printBinaryFile("output.bin");
    printRuns(filesCounter);

    cout << "\nStatystyki Dysku:" << endl;
    cout << "Odczyty stron: " << disk.read_count << endl;
    cout << "Zapisy stron: " << disk.write_count << endl;
}