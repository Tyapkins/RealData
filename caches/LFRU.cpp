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
    unsigned long size = 0;
    long occupied = 0;
    long all_bytes = 0, hit_bytes = 0;

protected:
    storage cach;

public:
    void setSize(long siz) { size = siz; }
    long getSize() {return size;}
    bool isOccupied(long reqsize) { return (occupied + reqsize) > size;}
    request remove_req();
    virtual void add_req(request req);
    request what_req();
    virtual bool find_req(request req);
    storage* get_cache() {return &cach;};
    float getBHR() {
        return float(hit_bytes)/all_bytes;};

    void add_occupied(long num) {occupied += num;};
    void add_all(long num) {all_bytes += num;};
    void add_hit(long num) {hit_bytes += num;};

};

request cache::what_req()
{
    request res;
    std::string item = "";
    long time_min = INT64_MAX, max_size = 0;
    for (auto min_search = cach.begin(); min_search != cach.end(); min_search++) {
        //std::cout << "time min is " << time_min << std::endl;
        time_min = (((*min_search).second).second < time_min) ? ((*min_search).second).second : time_min;
    }
    for (auto el = cach.begin(); el != cach.end(); el++)
    {
        if (((*el).second.second == time_min) && ((*el).second.first > max_size)) {
            item = (*el).first;
            max_size = (*el).second.first;
        }
        //std::cout << "max_size is " << max_size << std::endl;
    }
    res.name = item;
    res.size = max_size;
    res.time = time_min;
    return res;
}

request cache::remove_req()
{
    request req = what_req();
    //std::cout << "Try to remove " << req.name << std::endl;
    cach.erase(req.name);
    occupied -= req.size;
    //std::cout << "Occupied now is " << occupied << std::endl;
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

class LRU: public cache
{};

class LFU: public cache
{
    public:

        virtual void add_req(request req);
        virtual bool find_req(request req);
};

void LFU::add_req(request req)
{
    cach[req.name] = attrs(req.size, 1);
    add_occupied(req.size);
}

bool LFU::find_req(request req)
{
    add_all(req.size);
    if (cach.find(req.name) != cach.end()) {
        add_hit(req.size);
        cach[req.name].second += 1;
        return true;
    }
    /*else
    {
        while (isOccupied(req.size)) {
            auto remreq = remove_req();
            std::cout << "Removing request = " << remreq.name << std::endl;
        }
        add_req(req);
    }*/
    return false;
}



class TwoCache
{
    LRU head;
    LFU tail;
    unsigned long size = 0;
    float part = 1.0;
    long all_bytes = 0, hit_bytes = 0;

public:
    void setSize(unsigned long siz) {size = siz;}
    void setPart(float p) {part = (p <= 1) ? p : 1/p;}
    void segSize() {
        head.setSize(part*size);
        tail.setSize(size-head.getSize());
        }
    void swap_req();
    bool find_req(request req);
    float getBHR() {
        return float(hit_bytes)/all_bytes;};

};

void TwoCache::swap_req() {
    auto head_req = head.remove_req();
    if (head_req.size < tail.getSize()) {
        while (tail.isOccupied(head_req.size)) {
            //std::cout << "Swapping?" << std::endl;
            tail.remove_req();
        }
        tail.add_req(head_req);
    }
}

bool TwoCache::find_req(request req)
{
    all_bytes += req.size;
    if (!(tail.find_req(req))) {
        if (!(head.find_req(req))) {
            if (req.size < head.getSize()) {
                while (head.isOccupied(req.size)) {
                    swap_req();
                    //std::cout << "Swapping req = " << req.name << std::endl;
                }
                head.add_req(req);
            }
            return false;
        }
    }
    hit_bytes += req.size;
    return true;
}




int main(int argc, char** argv) {
    std::string path = (argc > 1) ? argv[1] : "big_compare.txt", buffer;
    int divisor = (argc > 2) ? atoi(argv[2]) : 10000;
    float p = (argc > 3) ? atof(argv[3]) : 0.5;
    std::ifstream requests(path);
    std::vector <std::string> split_request(3);
    TwoCache LFRU;
    std::cout.precision(8);
    if (argc > 4)
        LFRU.setSize(atol(argv[4]));
    else
        LFRU.setSize(5ULL*1024*1024*1024);
    long count = 0;
    LFRU.setPart(p);
    LFRU.segSize();
    while (std::getline(requests, buffer)) {
        if (!(buffer.empty())) {
            for (int i = 0; i < 3; i++) {
                split_request[i] = buffer.substr(0, (buffer.find(',')));
                buffer = buffer.substr(buffer.find(',') + 1);
            }
            LFRU.find_req(request(split_request));
            if ((++count)%divisor == 0)
                std::cout << std::fixed << count << " : " << LFRU.getBHR() << std::endl;
        }
    }
    std::cout << std::fixed << count << " : " << LFRU.getBHR() << std::endl;
    return 0;
}
