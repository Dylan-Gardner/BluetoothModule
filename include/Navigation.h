#ifndef Navigation_h
#define Navigation_h

class Navigation {
    private:
        char *distance, *next_distance, *time, *next_dir, *next_road;
        bool running;
    public: 
        Navigation(char* d, char* nd, char* t, char* ndir,char *nr ,bool r): distance(d), next_distance(nd), time(t), next_dir(ndir),next_road(nr) ,running(r){};
        ~Navigation(){};
        char* getDistance(){return distance;}
        char* getNextDistance(){return next_distance;}
        char* getNextDir(){return next_dir;}
        char* getNextRoad(){return next_road;}
        char *getTime(){return time;}
        bool getRunning(){return running;}
        void setDistance(char* d){distance = d;}
        void setNextDistance(char* nd){next_distance = nd;}
        void setNextDir(char* nd){next_dir = nd;}
        void setNextRoad(char* nr){next_road = nr;}
        void setTime(char* t){time = t;}
        void setRunning(bool r){running = r;}
        
};

#endif