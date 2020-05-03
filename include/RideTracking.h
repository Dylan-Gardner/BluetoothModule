#ifndef RideTracking_h
#define RideTracking_h

class RideTracking {
    private:
        char *time, *distance, *speed, *avg_speed, *gain;
        bool running;
    public: 
        RideTracking(char *d, char* s, char* a, char* g,char *t ,bool r): distance(d), speed(s), avg_speed(a), gain(g),time(t) ,running(r){};
        ~RideTracking(){};
        char* getDistance(){return distance;}
        char *getSpeed(){return speed;}
        char *getAvg(){return avg_speed;}
        char *getGain(){return gain;}
        char *getTime(){return time;}
        bool getRunning(){return running;}
        void setDistance(char* d){distance = d;}
        void setSpeed(char* s){speed = s;}
        void setAvg(char* a){avg_speed = a;}
        void setGain(char* g){gain = g;}
        void setTime(char* t){time = t;}
        void setRunning(bool r){running = r;}
        
};

#endif