#include "sim800.h"

/* Error codes */

enum Result {
  OK,
  ERROR_BEARER_PROFILE_GPRS,
  ERROR_BEARER_PROFILE_APN,
  ERROR_OPEN_GPRS_CONTEXT,
  ERROR_QUERY_GPRS_CONTEXT,
  ERROR_CLOSE_GPRS_CONTEXT,
  ERROR_HTTP_INIT,
  ERROR_HTTP_CID,
  ERROR_HTTP_PARA,
  ERROR_HTTP_GET,
  ERROR_HTTP_READ,
  ERROR_HTTP_CLOSE
};


class HTTP : public SIM800 {

  public:
    HTTP(int baudRate = 9600):SIM800(baudRate){};
    Result configureBearer(const char *apn);
    Result connect();
    Result disconnect();
    Result get(const char *url, char *response);

  private:
    void parseJSONResponse(const char *buffer, char *response, unsigned int bufferSize);
};