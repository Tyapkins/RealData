#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>

typedef std::pair<long, std::pair<long, long>> attrs;
typedef std::map <std::string, attrs> storage;

struct request
        : public std::basic_string<char> {
    std::string name;
    long size = 0;
    long time = 0;
    long next_time = 0;

public:
    request(std::vector <std::string> req)
    {
        if (req.size() >= 3)
        {
            name = req[0];
            size = stol(req[1]);
            time = stol(req[2]);
            if (req.size() >= 4)
                next_time = stol(req[3]);
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
    long time_max = 0, max_size = 0;
    for (auto max_search = cach.begin(); max_search != cach.end(); max_search++)
    {
        auto time_pair = (*max_search).second.second;
        if (time_pair.second == -1)
        {
                time_max = -1;
                break;
        }
        else
        {
                time_max = (time_pair.second - time_pair.first > time_max) ? (time_pair.second - time_pair.first) : time_max;
        }
    }
    std::string item_to_remove;
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        auto time_pair = (*el).second.second;
        auto val = (time_max >= 0) ? time_pair.second - time_pair.first : time_pair.second;
        if ((val == time_max) && ((*el).second.first > max_size)) {
            item_to_remove = (*el).first;
            max_size = (*el).second.first;
        }
    }
    cach.erase(item_to_remove);
    occupied -= max_size;
}

void cache::add_req(request req)
{
    cach[req.name] = attrs(req.size, std::pair(req.time, req.next_time));
    occupied += req.size;
}

bool cache::find_req(request req)
{
    all_bytes += req.size;
    all_objects++;
    if (cach.find(req.name) != cach.end()) {
        hit_bytes += req.size;
        hit_objects++;
        cach[req.name].second.first = req.time;
        cach[req.name].second.second = req.next_time;
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
    std::vector <std::string> split_request(4);
    cache LRU;
    std::cout.precision(8);
    if (argc > 3)
        LRU.setSize(atol(argv[3]));
    else
        LRU.setSize(5ULL*1024*1024*1024);
    long count = 0;
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 4; i++) {
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
