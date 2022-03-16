/*
 * Servidor FTP para ESP8266
 * basado en FTP Serveur para Arduino Due y Ethernet shield (W5100) o WIZ820io (W5200)
 * basado en el trabajo de Jean-Michel Gallego
 * modificado para trabajar con esp8266 SPIFFS por David Paiva david@nailbuster.com
 * modificado para trabajar con esp8266 SD por "https://trionking.blogspot.com/2017/02/esp8266-wifi-ftp-server-with-sd.html"
 * modificado para trabajar con esp8266 SD sin modificar la biblioteca SD por Srinivasan https://Github.com/Neutrino-1
 
 * Este programa es software gratuito: puedes redistribuirlo y / o modificar
 * bajo los términos de la Licencia Pública General GNU publicada por
 * Free Software Foundation, ya sea la versión 3 de la Licencia, o
 * (a su elección) cualquier versión posterior.
 *
 * Este programa se distribuye con la esperanza de que sea útil,
 * pero SIN NINGUNA GARANTÍA; sin siquiera la garantía implícita de
 * COMERCIABILIDAD o APTITUD PARA UN PROPÓSITO PARTICULAR. Ver el
 * Licencia pública general GNU para más detalles.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*******************************************************************************
 **                                                                            **
 **                     DEFINICIONES PARA EL SERVIDOR FTP                      **
 **                                                                            **
 *******************************************************************************/

// Descomente para imprimir la información de depuración en la consola adjunta a ESP8266
//#define FTP_DEBUG

#ifndef FTP_SERVERESP_H
#define FTP_SERVERESP_H

//#include "Streaming.h"
#include <FS.h>
#include <WiFiClient.h>
#include <SD.h>
#define FTP_SERVER_VERSION "FTP-2017-10-18"

#define FTP_CTRL_PORT    21          // Command port on wich server is listening  
#define FTP_DATA_PORT_PASV 50009     // Data port in passive mode

#define FTP_TIME_OUT  5           // Disconnect client after 5 minutes of inactivity
#define FTP_CMD_SIZE 255 + 8 // max size of a command
#define FTP_CWD_SIZE 255 + 8 // max size of a directory name
#define FTP_FIL_SIZE 255     // max size of a file name
//#define FTP_BUF_SIZE 1024 //512   // size of file buffer for read/write
#define FTP_BUF_SIZE 2*1460 //512   // size of file buffer for read/write

class FtpServer
{
public:
  void    begin(String uname, String pword);
  void    handleFTP();

private:
  void    iniVariables();
  void    clientConnected();
  void    disconnectClient();
  boolean userIdentity();
  boolean userPassword();
  boolean processCommand();
  boolean dataConnect();
  boolean doRetrieve();
  boolean doStore();
  void    closeTransfer();
  void    abortTransfer();
  boolean makePath( char * fullname );
  boolean makePath( char * fullName, char * param );
  uint8_t getDateTime( uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                       uint8_t * phour, uint8_t * pminute, uint8_t * second );
  char *  makeDateTimeStr( char * tstr, uint16_t date, uint16_t time );
  int8_t  readChar();

  IPAddress      dataIp;              // IP address of client for data
  WiFiClient client;
  WiFiClient data;
  
  File file;
  
  boolean  dataPassiveConn;
  uint16_t dataPort;
  char     buf[ FTP_BUF_SIZE ];       // data buffer for transfers
  char     cmdLine[ FTP_CMD_SIZE ];   // where to store incoming char from client
  char     cwdName[ FTP_CWD_SIZE ];   // name of current directory
  char     command[ 5 ];              // command sent by client
  boolean  rnfrCmd;                   // previous command was RNFR
  char *   parameters;                // point to begin of parameters sent by client
  uint16_t iCL;                       // pointer to cmdLine next incoming char
  int8_t   SD_Status,
           cmdStatus,                 // status of ftp command connexion
           transferStatus;            // status of ftp data transfer
  uint32_t millisTimeOut,             // disconnect after 5 min of inactivity
           millisDelay,
           millisEndConnection,       // 
           millisBeginTrans,          // store time of beginning of a transaction
           bytesTransfered;           //
  String   _FTP_USER;
  String   _FTP_PASS;

  

};

#endif // FTP_SERVERESP_H
