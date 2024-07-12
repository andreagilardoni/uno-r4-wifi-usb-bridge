#ifndef ARDUINO_AT_HANDLER_H
#define ARDUINO_AT_HANDLER_H

#include "commands.h"
#include "chAT.hpp"
#include "WiFi.h"
#include "Server.h"

#include "WiFiClient.h"
#include <WiFiClientSecure.h>
#include "FS.h"
#include "SPIFFS.h"

#define MAX_CLIENT_AVAILABLE   8
#define MAX_SERVER_AVAILABLE   4
#define MAX_UDP_AVAILABLE      4

#define ESP_FW_VERSION    "0.4.1"
#define FIRMWARE_MAYOR    0
#define FIRMWARE_MINOR    4
#define FIRMWARE_PATCH    1

#define GPIO_BOOT   9
#define GPIO_RST    4

using namespace SudoMaker;

#define WIFI_ST_NO_SHIELD            255
#define WIFI_ST_NO_MODULE            255
#define WIFI_ST_IDLE_STATUS           0
#define WIFI_ST_NO_SSID_AVAIL         1
#define WIFI_ST_SCAN_COMPLETED        2
#define WIFI_ST_CONNECTED             3
#define WIFI_ST_CONNECT_FAILED        4
#define WIFI_ST_CONNECTION_LOST       5
#define WIFI_ST_DISCONNECTED          6
#define WIFI_ST_AP_LISTENING          7
#define WIFI_ST_AP_CONNECTED          8
#define WIFI_ST_AP_FAILED             9


class CClientWrapper {
public:
   WiFiClient *client;
   WiFiClientSecure *sslclient;
   int can_delete = -1;
};

class CServerClient {
public:
   WiFiClient client;
   int server = -1;
   bool accepted = false;
};

class CAtHandler {
private:
   static uint8_t wifi_status;

   int last_server_client_sock;

   WiFiUDP    * udps[MAX_UDP_AVAILABLE];
   WiFiServer * serverWiFi[MAX_SERVER_AVAILABLE];
   WiFiClient * clients[MAX_CLIENT_AVAILABLE];
   CServerClient  serverClients[MAX_CLIENT_AVAILABLE];
   WiFiClientSecure * sslclients[MAX_CLIENT_AVAILABLE];
   std::vector<std::uint8_t> clients_ca[MAX_CLIENT_AVAILABLE];
   std::vector<std::uint8_t> clients_cert_pem[MAX_CLIENT_AVAILABLE];
   std::vector<std::uint8_t> clients_key_pem[MAX_CLIENT_AVAILABLE];
   int udps_num = 0;
   int servers_num = 0;
   int clientsToServer_num = 0;
   int clients_num = 0;
   int sslclients_num = 0;

   // TODO put this in commands.h
   static constexpr std::array command_names{
      _WIFISCAN, _RESET, _RESTART_BOOTLOADER, _GMR, _GENERIC_CMD, _FILESYSTEM, _MOUNTFS, _EXIT, _MODE, _BEGINSTA, _GETSTATUS,
      _RECONNECT, _DISCONNECT, _BEGINSOFTAP, _MACSTA, _MACSOFTAP, _DISCONNECTSOFTAP, _AUTOCONNECT, _IPSTA, _IPSOFTAP, _IPV6,
      _GETRSSI, _GETSSID, _GETBSSID, _GETSOFTAPSSID, _HOSTNAME, _BEGINCLIENT, _CLIENTSTATE, _CLIENTCONNECTIP, _CLIENTCONNECTNAME,
      _CLIENTCONNECT, _CLIENTSEND, _CLIENTRECEIVE, _CLIENTCLOSE, _IPCLIENT, _BEGINSERVER, _CLIENTCONNECTED, _SSLBEGINCLIENT,
      _SETCAROOT, _SETECCSLOT, _SSLCLIENTSTATE, _SSLCLIENTCONNECTNAME, _SSLCLIENTCONNECT, _SETIP, _GETHOSTBYNAME, _AVAILABLE,
      _PEEK, _CLIENTFLUSH, _REMOTEIP, _REMOTEPORT, _CLIENTSTATUS, _SOFTRESETWIFI, _SSLCLIENTCONNECTIP, _SSLCLIENTSEND,
      _SSLCLIENTCLOSE, _SSLIPCLIENT, _SSLCLIENTCONNECTED, _SSLCLIENTRECEIVE, _SSLAVAILABLE, _SSLCLIENTSTATUS, _SSLCLIENTFLUSH,
      _SSLREMOTEIP, _SSLREMOTEPORT, _SSLPEEK, _SERVERAVAILABLE, _SERVERACCEPT, _SERVEREND, _UDPBEGIN, _UDPBEGINMULTI, _UDPBEGINPACKET,
      _UDPBEGINPACKETMULTI, _UDPBEGINPACKETNAME, _UDPBEGINPACKETIP, _UDPENDPACKET, _UDPWRITE, _UDPPARSE, _UDPAVAILABLE, _UDPREAD,
      _UDPPEEK, _UDPFLUSH, _UDPREMOTEIP, _UDPREMOTEPORT, _UDPSTOP, _FWVERSION, _SOFTAPCONFIG, _SERVERWRITE, _HCI_BEGIN, _HCI_END,
      _HCI_WAIT, _HCI_READ, _HCI_WRITE, _HCI_AVAILABLE, _OTA_SETCAROOT, _OTA_BEGIN, _OTA_DOWNLOAD, _OTA_VERIFY, _OTA_UPDATE,
      _OTA_RESET, _PREF_BEGIN, _PREF_END, _PREF_CLEAR, _PREF_REMOVE, _PREF_PUT, _PREF_GET, _PREF_LEN, _PREF_STAT, _SOFTSE_BEGIN,
      _SOFTSE_END, _SOFTSE_SERIAL, _SOFTSE_RND, _SOFTSE_PRI_KEY, _SOFTSE_PUB_KEY, _SOFTSE_WRITE_SLOT, _SOFTSE_READ_SLOT,
      _SOFTSE_S_V_BUF_SET, _SOFTSE_SIGN_GET, _SOFTSE_VERIFY_GET, _SOFTSE_SHA256_GET,
   };

   std::array<std::function<chAT::CommandStatus(chAT::Server&, chAT::ATParser&)>, command_names.size()> command_table;

   chAT::Server at_srv;
   HardwareSerial *serial;
   uint8_t* cert_in_flash_ptr = nullptr;
   spi_flash_mmap_handle_t cert_in_flash_handle;

   CClientWrapper getClient(int sock);

   chAT::CommandStatus esp_generic_reset(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_restart_bootloader(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_gmr(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_generic_cmd(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_fw_version(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_filesystem(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_mountfs(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_exit(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus esp_generic_soft_reset_wifi(chAT::Server&, chAT::ATParser&);
   void add_cmds_esp_generic();
   void add_cmds_wifi_station();
   void add_cmds_wifi_softAP();
   void add_cmds_wifi_SSL();
   void add_cmds_wifi_netif();
   void add_cmds_wifi_udp();

   chAT::CommandStatus ble_bridge_hci_begin(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus ble_bridge_hci_end(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus ble_bridge_hci_wait(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus ble_bridge_hci_available(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus ble_bridge_hci_read(chAT::Server&, chAT::ATParser&);
   chAT::CommandStatus ble_bridge_hci_write(chAT::Server&, chAT::ATParser&);
   void add_cmds_ble_bridge();
   void add_cmds_ota();
   void add_cmds_preferences();
   void add_cmds_se();
public:
   /* Used by cmds_se */
   std::vector<std::uint8_t> se_buf;

   /* Used by cmds_ota */
   std::vector<std::uint8_t> ota_cert_buf;

   /* Used by cmds_preferences */
   std::vector<std::uint8_t> pref_buf;

   CAtHandler(HardwareSerial *s);
   CAtHandler() = delete ;
   static void onWiFiEvent(WiFiEvent_t event);
   void run();
};


#endif
