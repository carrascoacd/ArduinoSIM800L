#include "sim800.h"
#include <string.h>
#include <ArduinoJson.h>

/* Result codes */
enum Result {
  SUCCESS,
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
  ERROR_HTTP_CLOSE,
  ERROR_HTTP_POST,
  ERROR_HTTP_DATA,
  ERROR_HTTP_CONTENT
};


class HTTP : public SIM800 {

  public:
    HTTP(int baudRate = 9600):SIM800(baudRate){};
    Result configureBearer(const char *apn);
    Result connect();
    Result disconnect();
    Result get(const char *uri, char *response);
    Result post(const char *uri, const char *body, char *response);

  private:
    void readResponse(char *response);
    Result setHTTPSession(const char *uri);
    void parseJSONResponse(const char *buffer, unsigned int bufferSize, char *response);
};