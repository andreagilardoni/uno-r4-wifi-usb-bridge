#include "at_handler.h"
#include "commands.h"
#include "cmds_esp_generic.h"


#include "cmds_wifi_station.h"
#include "cmds_wifi_softAP.h"
#include "cmds_wifi_netif.h"
#include "cmds_wifi_SSL.h"

using namespace SudoMaker;

/* -------------------------------------------------------------------------- */
CClientWrapper CAtHandler::getClient(int sock) {
/* -------------------------------------------------------------------------- */
   CClientWrapper rv;
   
   bool is_server = false;
   bool is_sslclienet = false;

   int internal_sock = -1;

   if(sock >= START_SSL_CLIENT_SOCK) {
      internal_sock = sock - START_SSL_CLIENT_SOCK;
      is_sslclienet = true;
   } else
   if(sock >= START_CLIENT_SERVER_SOCK) {
      internal_sock = sock - START_CLIENT_SERVER_SOCK;
      is_server = true;
   }
   else {
      internal_sock = sock;
   }

   if(internal_sock < 0 || internal_sock >= MAX_CLIENT_AVAILABLE) {
      rv.client = nullptr;
      rv.sslclient = nullptr;
      rv.can_delete = -1;
      return rv;
   }

   if (is_sslclienet) {
    rv.sslclient = sslclients[internal_sock];
    rv.can_delete = internal_sock;
   }
   else if(is_server) {
      rv.client = &serverClients[internal_sock];
      rv.can_delete = -1;
   }
   else {
      rv.client = clients[internal_sock];
      rv.can_delete = internal_sock;
   }
   return rv;
}



/* -------------------------------------------------------------------------- */
void CAtHandler::run() {
/* -------------------------------------------------------------------------- */   
   at_srv.run();
}

/* -------------------------------------------------------------------------- */
void CAtHandler::onWiFiEvent(WiFiEvent_t event) {
/* -------------------------------------------------------------------------- */   
   switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      //SERIAL_DEBUG.println("Station Mode Started");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //SERIAL_DEBUG.println("Connected to :" + String(WiFi.SSID()));
      //SERIAL_DEBUG.print("Got IP: ");
      //SERIAL_DEBUG.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      //SERIAL_DEBUG.println("Disconnected from station, attempting reconnection");
      //WiFi.reconnect();
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      //SERIAL_DEBUG.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      //wpsStop();
      delay(10);
      //SERIAL_DEBUG.begin();
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      //SERIAL_DEBUG.println("WPS Failed, retrying");
      //wpsStop();
      //wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      //SERIAL_DEBUG.println("WPS Timedout, retrying");
      //wpsStop();
      //wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_PIN:
      //Serial.println("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code));
      break;
    default:
      break;
  }

}

/* -------------------------------------------------------------------------- */
CAtHandler::CAtHandler(HardwareSerial *s)  {
/* -------------------------------------------------------------------------- */   
   
  for(int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
    clients[i] = nullptr;
  }

  for(int i = 0; i < MAX_SERVER_AVAILABLE; i++) {
    serverWiFi[i] = nullptr;
  }

  /* Set up wifi event */
  WiFi.onEvent(onWiFiEvent);
   
  /* set up serial */
  serial = s;

  /* set up chatAt server callbacks */
  at_srv.set_io_callback({
    .callback_io_read = [this](auto buf, auto len) {
      if (!serial->available()) {
        yield();
        return (unsigned int)0;
      }
      return serial->read(buf, len);
    },
    .callback_io_write = [this](auto buf, auto len) {
      return serial->write(buf, len);
    },
  });

  at_srv.set_command_callback([this](chAT::Server & srv, const std::string & command) {
    auto it = command_table.find(command);

    if (it == command_table.end()) {
      return chAT::CommandStatus::ERROR;
    } 
    else {
      return it->second(srv, srv.parser());
    }
  });

  /*  SET UP COMMAND TABLE */ 
  add_cmds_esp_generic();
  add_cmds_wifi_station();
  add_cmds_wifi_softAP(); 
  add_cmds_wifi_SSL();
  add_cmds_wifi_netif();
}