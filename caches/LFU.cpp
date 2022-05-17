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
    long times = 0;

public:
    request(std::vector <std::string> req)
    {
        if (req.size() >= 2)
        {
            name = req[0];
            size = stoi(req[1]);
        }
    }
};


class cache
{
    storage cach;
    unsigned long size = 0;
    long occupied = 0;
    long all_bytes = 0, hit_bytes = 0;
    long all_objects = 0, hit_objects = 0;

public:
    void setSize(unsigned long siz) { size = siz; }
    bool isOccupied(long reqsize) { return (occupied + reqsize) > size;}
    void remove_req();
    void add_req(request req);
    bool find_req(request req);
    float getBHR() {
        return float(hit_bytes)/all_bytes;};
    float getOHR() {
        return float(hit_objects)/all_objects;};

};

void cache::remove_req()
{
    long times_min = INT64_MAX, max_size = 0;
    for (auto min_search = cach.begin(); min_search != cach.end(); min_search++)
        times_min = (((*min_search).second).second < times_min) ? ((*min_search).second).second : times_min;
    //std::cout << time_min << " is time_min " << std::endl;
    std::string item_to_remove;
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        if (((*el).second.second == times_min) && ((*el).second.first > max_size)) {
            item_to_remove = (*el).first;
            max_size = (*el).second.first;
        }
    }
    cach.erase(item_to_remove);
    occupied -= max_size;
}

void cache::add_req(request req)
{
    cach[req.name] = attrs(req.size, req.times);
    occupied += req.size;
}

bool cache::find_req(request req)
{
    all_bytes += req.size;
    all_objects++;
    if (cach.find(req.name) != cach.cend()) {
        hit_bytes += req.size;
        hit_objects++;
        cach[req.name].second += 1;
        return true;
    }
    else
    {
        if (req.size < size)
        {
           while (isOccupied(req.size)) {
                remove_req();
           }
           add_req(req);
        }
    }
    return false;
}


int main(int argc, char** argv) {
    std::string path = (argc > 1) ? argv[1] : "big_compare.txt", buffer;
    int divisor = (argc > 2) ? atoi(argv[2]) : 10000;
    std::ifstream requests(path);
    std::vector <std::string> split_request(3);
    cache LRU;
    std::cout.precision(8);
    if (argc > 3)
        LRU.setSize(atol(argv[3]));
    else
        LRU.setSize(5ULL*1024*1024*1024);
    long count = 0;
    //int all_bytes = 0, hit_bytes = 0;
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 3; i++) {
                split_request[i] = buffer.substr(0, (buffer.find(',')));
                buffer = buffer.substr(buffer.find(',') + 1);
            }
            LRU.find_req(request(split_request));
            if ((++count)%divisor == 0)
                std::cout << std::fixed << count << " : " << LRU.getBHR() << " : " << LRU.getOHR() << std::endl;
        }
    }
    std::cout << std::fixed << count << " : " << LRU.getBHR() << " : " << LRU.getOHR() << std::endl;
    return 0;
}
