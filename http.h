#include "sim800.h"


class HTTP : public SIM800 {

  public:
    
    HTTP(int baudRate = 9600):SIM800(baudRate){};
    void configureBearer(const char *apn);
    void connect();
    void disconnect();
    void get(const char *url, char *response);
};