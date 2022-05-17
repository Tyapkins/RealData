#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>

typedef std::pair<long, long> attrs;
typedef std::map <std::string, attrs> storage;

struct request
        : public std::basic_string<char> {
    std::string name;
    long size = 0;
    long time = 0;

public:
    request() {name = "";};

    request(std::vector <std::string> req)
    {
        if (req.size() >= 3)
        {
            name = req[0];
            size = stoi(req[1]);
            time = stoi(req[2]);
        }
    }
};


class cache
{
    storage cach;
    unsigned long size = 0;
    long occupied = 0;
     long all_bytes = 0, hit_bytes = 0;

public:
    void setSize(unsigned long siz) { size = siz; }
    long getSize() {return size;}
    bool isOccupied(long reqsize) { return (occupied + reqsize) > size;}
    request remove_req();
    void add_req(request req);
    request what_req();
    bool find_req(request req);
    storage* get_cache() {return &cach;};
    float getBHR() {
        return float(hit_bytes)/all_bytes;};

};

request cache::what_req()
{
    request res;
    std::string item = "";
    long time_min = INT64_MAX, max_size = 0;
    for (auto min_search = cach.begin(); min_search != cach.end(); min_search++)
        time_min = (((*min_search).second).second < time_min) ? ((*min_search).second).second : time_min;
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        if (((*el).second.second == time_min) && ((*el).second.first > max_size)) {
            item = (*el).first;
            max_size = (*el).second.first;
        }
    }
    res.name = item;
    res.size = max_size;
    res.time = time_min;
    return res;
}

request cache::remove_req()
{
    request req = what_req();
    cach.erase(req.name);
    occupied -= req.size;
    return req;
}

void cache::add_req(request req)
{
    cach[req.name] = attrs(req.size, req.time);
    occupied += req.size;
}

bool cache::find_req(request req)
{
    all_bytes += req.size;
    if (cach.find(req.name) != cach.end()) {
        hit_bytes += req.size;
        cach[req.name].second = req.time;
        return true;
    }
    return false;
}

class SegCache
{
    int K;
    std::vector <cache> segments;
    unsigned long size;
    std::vector <float> sizes;
    long all_bytes = 0, hit_bytes = 0;

public:
    void setSegNum(int num) {K = num;}
    void setSize(long siz) {size = siz;}
    void distrSize(std::vector <float> nums) {sizes = nums;}
    void segSize() {
        long sum = 0;
        segments = std::vector <cache>(K);
        for (int i = 0; i < K-1; i++) {
            segments.at(i).setSize(sizes[i] * size);
            sum += sizes.at(i)*size;
        }
        segments.at(K-1).setSize(size-sum);
        }
    bool find_req(request req);
    float getBHR() {
        return float(hit_bytes)/all_bytes;};

};

bool SegCache::find_req(request req)
{
    all_bytes += req.size;
    for (int i = 0; i < K; i++)
    {
        if (segments[i].find_req(req)) {
            hit_bytes += req.size;
            return true;
        }
    }
    for (int i = 0; i < K; i++)
    {
        if (req.size < segments[i].getSize()) {
            if (!(segments[i].isOccupied(req.size)))
            {
                segments[i].add_req(req);
                return false;
            }
        }
    }
    int j = rand()%K;;
    while (req.size >= segments[j].getSize())
        j = rand()%K;
    while (segments[j].isOccupied(req.size))
        segments[j].remove_req();
    segments[j].add_req(req);
    return false;
}




int main(int argc, char** argv) {
    std::string path = (argc > 1) ? argv[1] : "big_compare.txt", buffer;
    int divisor = (argc > 2) ? atoi(argv[2]) : 10000;
    std::vector <float> nums = {0.4, 0.2, 0.2, 0.2};
    if (argc > 4) {
        nums.clear();
        for (int i = 4; i < argc; i++)
            nums.push_back(std::stof(argv[i]));
    }
    std::ifstream requests(path);
    std::vector <std::string> split_request(3);
    SegCache LRU;
    std::cout.precision(8);
    if (argc > 3)
        LRU.setSize(std::stol(argv[3]));
    else
        LRU.setSize(80ULL*1024*1024*1024);
    long count = 0;
    LRU.setSegNum(nums.size());
    LRU.distrSize(nums);
    LRU.segSize();
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 3; i++) {
                split_request[i] = buffer.substr(0, (buffer.find(',')));
                buffer = buffer.substr(buffer.find(',') + 1);
            }
            LRU.find_req(request(split_request));
            if ((++count)%divisor == 0)
                std::cout << std::fixed << count << " : " << LRU.getBHR() << std::endl;
        }
    }
    std::cout << std::fixed << count << " : " << LRU.getBHR() << std::endl;
    return 0;
}
