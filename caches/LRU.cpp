#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>

typedef std::pair<long, long> attrs;
//typedef std::vector <std::string> radswed;
typedef std::map <std::string, attrs> storage;

struct request
        : public std::basic_string<char> {
    std::string name;
    long size = 0;
    long time = 0;

public:
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
    long all_objects = 0, hit_objects = 0;

public:
    void setSize(unsigned long siz) { size = siz; }
    long getSize() {return size;}
    bool isOccupied(long reqsize) { return (occupied + reqsize) > size;}
    void remove_req();
    void add_req(request req);
    bool find_req(request req);
    float getBHR() {
        return float(hit_bytes)/all_bytes;};
    float getOHR() {
        return float(hit_objects)/all_objects;};
    long getAll() {return all_bytes;}
    long getHit() {return hit_bytes;}

};

void cache::remove_req()
{
    long time_min = INT64_MAX, max_size = 0;
    for (auto min_search = cach.begin(); min_search != cach.end(); min_search++)
        time_min = (((*min_search).second).second < time_min) ? ((*min_search).second).second : time_min;
    std::string item_to_remove;
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        if (((*el).second.second == time_min) && ((*el).second.first > max_size)) {
            item_to_remove = (*el).first;
            max_size = (*el).second.first;
        }
    }
    cach.erase(item_to_remove);
    occupied -= max_size;
}

void cache::add_req(request req)
{
    cach[req.name] = attrs(req.size, req.time);
    occupied += req.size;
}

bool cache::find_req(request req)
{
    all_bytes += req.size;
    all_objects++;
    if (cach.find(req.name) != cach.end()) {
        hit_bytes += req.size;
        hit_objects++;
        cach[req.name].second = req.time;
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
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 3; i++) {
                split_request[i] = buffer.substr(0, (buffer.find(',')));
                buffer = buffer.substr(buffer.find(',') + 1);
            }
            request tyapka(split_request);
            /*std::cout << "Name: " << tyapka.name << std::endl;
            std::cout << "Size: " << tyapka.size << std::endl;
            std::cout << "Time: " << tyapka.time << std::endl;*/
            LRU.find_req(request(split_request));
            //std::cout << LRU.getAll() << "/" << LRU.getHit() << std::endl;
            if ((++count)%divisor == 0)
                std::cout << std::fixed << count << " : " << LRU.getBHR() << " : " << LRU.getOHR() << std::endl;
        }
    }
    std::cout << std::fixed << count << " : " << LRU.getBHR() << " : " << LRU.getOHR() << std::endl;
    return 0;
}
