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
#include <cstdio>
using namespace std;

# define RECORDSIZE 24

//sorting by heat
struct Record{
    double mass = 0.0;
    double specific_heat = 0.0;
    double temperature_difference = 0.0;
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
                index=smallest;
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

//reading and printing data
void printBinaryFile(const string& filename) {
    cout << "\n--- Reading binary file: " << filename << " ---\n";
    fstream file(filename, ios::in | ios::binary);
    Record r{};
    int i = 0;
    while (file.read(reinterpret_cast<char*>(&r), sizeof(Record))) {
        if (r.mass>0)
            cout << "[" << i++ << "] M=" << fixed << setprecision(4) << r.mass
                 << " C=" << r.specific_heat
                 << " dT=" << r.temperature_difference
                 << " | Q=" << r.heat() << endl;
              //  cout<<r.mass<<" "<<r.specific_heat<<" "<<r.temperature_difference<<endl;
    }
    cout << "-----------------------------------\n";
}
void printRuns(const int filesCounter) {
    cout << "\n--- Reading Runs files: "  << " ---\n";
    for (int i=0; i<filesCounter; ++i) {
        string outputName = "run_" + to_string(i) + ".bin";
        fstream file(outputName, ios::in | ios::binary);
        Record r{};
        int iter = 0;
        while (file.read(reinterpret_cast<char*>(&r), sizeof(Record))) {
            if (r.mass>0)
            cout << "[runnumber: " << i << " recorditerator: "<<iter++<<" ] ""M=" << fixed << setprecision(4) << r.mass
                 << " C=" << r.specific_heat
                 << " dT=" << r.temperature_difference
                 << " | Q=" << r.heat() << endl;
        }
    }
}
vector<Record> generateRandomRecords(const int n_records) {
    cout << "Generating " << n_records << " random records in memory..." << endl;
    vector<Record> records;
    records.reserve(n_records);

    mt19937 gen(random_device{}());
    uniform_real_distribution<> mass_dist(1.0, 100.0);
    uniform_real_distribution<> heat_dist(0.1, 5.0);
    uniform_real_distribution<> temp_dist(-50.0, 50.0);

    for (int i = 0; i < n_records; ++i) {
        records.push_back(Record{mass_dist(gen), heat_dist(gen), temp_dist(gen)});
    }
    return records;
}
void writeVectorToBinaryFile(const string& filename, const vector<Record>& records) {
    cout << "Writing " << records.size() << " records to binary file: " << filename << endl;
    fstream file(filename, ios::out | ios::binary | ios::trunc);

    for (const auto& r : records) {
        file.write(reinterpret_cast<const char*>(&r), RECORDSIZE);
    }
    file.close();

    ifstream test(filename, ios::binary | ios::ate);
    long long size = test.tellg();
    cout << "File size: " << size << " bytes." << endl;
    cout << "Expected: " << (long long)records.size() * RECORDSIZE << " bytes." << endl;
}
vector<Record> readRecordsFromKeyboard() {
    cout << "Enter records(3 doubles) separeted by a space" << endl;
    cout << "enter letter to end" << endl;
    vector<Record> records;
    Record r;

    while (true) {
        cout << "Enter [mass], [specific heat], [temperature_difference] ";
        if (!(cin >> r.mass >> r.specific_heat >> r.temperature_difference) || r.mass == 0) {
            break;
        }
        records.push_back(r);
    }
    cin.clear();
    cin.ignore(10000, '\n');
    return records;
}

vector<Record> readRecordsFromTxtFile(const string& txt_filename) {
    vector<Record> records;
    fstream file(txt_filename, ios::in);

    Record r;
    while (file >> r.mass >> r.specific_heat >> r.temperature_difference) {
        records.push_back(r);
    }
    file.close();
    return records;
}


//ALGORITHM PART
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
                    if (j.mass > 0) {
                        ram.push_back(j);
                    }
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
        //if smth wasn't pushed to disk
        if (!connector.empty()) {
            while (connector.size() < b) {
                connector.push_back(Record{});
            }
            disk.writePage(file_output, connector);
        }
        file_output.close();

    }
    file.close();
    cout << "\nDo you want to view files after creating Runs? (t/n)  ";
    char choice;
    cin >> choice;
    if (choice == 't' || choice == 'T')
        printRuns(files_counter);
    return files_counter;
}
//part2
void mergeRuns(DiskManager&disk,int nofBuffers,vector<string>filesBatch,const string& outputName) {
    //files counter is number of runs
    int nofmergingbuffers=filesBatch.size();

    //opening input files
    vector<unique_ptr<fstream>> input_files;
    for (int i=0; i<nofmergingbuffers; ++i) {
        input_files.push_back( make_unique<fstream>(filesBatch[i], ios::in | ios::binary));
    }

    //opening output files
    fstream outputfile(outputName,ios::out|ios::binary|ios::trunc);
    //buffer holds 1 page
    vector<vector<Record>> mergingBuffers(nofmergingbuffers);
    vector<Record> outputBuffer;
    //vector holds
    vector<int> recordInBufferIterator(nofmergingbuffers,1);
    MinHeap heap;
    int b=disk.getB();

    //initialize heap and merging buffers
    for (int i=0; i<nofmergingbuffers; ++i) {
        if (disk.readPage(*input_files[i],mergingBuffers[i])) {
            if (mergingBuffers[i][0].mass > 0) {
                heap.push({mergingBuffers[i][0],i});
                recordInBufferIterator[i]=1;
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

        //push to output buffer if output buffer is full
        outputBuffer.push_back(r);
        if (outputBuffer.size()==b) {
            disk.writePage(outputfile,outputBuffer);
            outputBuffer.clear();
        }

//reading next page with skipping the padding
        while (true) {
            //if one of the merging buffer is fully seen we read next page from disk
            if (recordInBufferIterator[run_index] >= b)
                {
                if (disk.readPage(*input_files[run_index], mergingBuffers[run_index])) {
                    recordInBufferIterator[run_index] = 0;
                }
                else
                    break;
            }
            if (mergingBuffers[run_index][recordInBufferIterator[run_index]].mass > 0) {
                heap.push({mergingBuffers[run_index][recordInBufferIterator[run_index]], run_index});
                recordInBufferIterator[run_index]++;
                break;
            }
            //if padding then skip this elemenet
            else {
                recordInBufferIterator[run_index]++;
            }
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
void deleteFiles(const vector<string>& files) {
    for (const string& file : files) {
        remove(file.c_str());
    }
}


int mergeManager(int filesCounter,int nofBuffers,DiskManager&disk ) {
    //files afrer creating runs
    vector<string> filesAvailable;
    for (int i=0 ; i<filesCounter; ++i) {
        filesAvailable.push_back("run_" + to_string(i) + ".bin");
    }
    int filesPossibleToMarge=nofBuffers-1;
    int phase=0;
    int outputCounter=0;
    vector<string> filesAfterMergingPhase;
    vector<string> filesCurrentlyMerged;
    while (filesAvailable.size()>1) {
        for (int j=0; j<filesAvailable.size(); j+=filesPossibleToMarge) {
            filesCurrentlyMerged.clear();

            for (int i=j; i<j+filesPossibleToMarge && i<filesAvailable.size(); ++i) {
                filesCurrentlyMerged.push_back(filesAvailable[i]);
            }
            string outputName="phase_" +to_string(phase) +"_run_"+to_string(outputCounter)+".bin";
            mergeRuns(disk,nofBuffers,filesCurrentlyMerged,outputName);
            filesAfterMergingPhase.push_back(outputName);
            outputCounter++;
        }
        deleteFiles(filesAvailable);
        filesAvailable=filesAfterMergingPhase;
        filesAfterMergingPhase.clear();
        phase++;
        cout << "\n--- Phase number: " << phase << " has ended---";

        cout << "\nDo you want to view files after this phase? (t/n)  ";
        char choice;
        cin >> choice;
        if (choice == 't' || choice == 'T') {
            for (const string& f : filesAvailable) {
                printBinaryFile(f);
            }
        }
    }

    remove("output.bin");

    rename(filesAvailable[0].c_str(), "output.bin");
    return phase;
}

int main() {
    long long n;
    int nofBuffors;
    int b;
    cout << "Enter:\n1. n (number of buffers available for sorting)\n2. b (blocking factor)  ";
    cout<<"\n1.n: ";
    cin>>nofBuffors;
    cout<<"\n2.b: ";
    cin>>b;

    vector<Record> recordsToProcess;
    cout << "\nSelect data source:\n";
    cout << "1. Generate random records (provide N)\n";
    cout << "2. Enter manually from keyboard\n";
    cout << "3. Read from file 'dane_testowe.txt'\n";
    cout << "Choice: ";
    int choice;
    cin >> choice;
    const string filename = "dane.bin";
    switch (choice) {
        case 1: {
            long long n;
            cout << "\nEnter N (number of records to generate): ";
            cin >> n;
            recordsToProcess = generateRandomRecords(n);
            break;
        }
        case 2:
            recordsToProcess = readRecordsFromKeyboard();
            break;
        case 3:
            recordsToProcess = readRecordsFromTxtFile("dane_testowe.txt");
            break;
        default:
            cout << "Invalid choice." << endl;
            return 1;
    }
    if (recordsToProcess.empty()) {
        cout << "No records loaded. Exiting program." << endl;
        return 0;
    }
    writeVectorToBinaryFile(filename, recordsToProcess);
    printBinaryFile(filename);

    DiskManager disk(b);
    const int filesCounter=createRuns(filename,disk,nofBuffors);
    int phase=mergeManager(filesCounter, nofBuffors, disk);
    printBinaryFile("output.bin");
 //   printRuns(filesCounter);

    cout << "\nSorting Stats:" << endl;
    cout << "Page readings: " << disk.read_count << endl;
    cout << "Page writings: " << disk.write_count << endl;
    cout<<"Number of phases: "<<phase<<endl;
}