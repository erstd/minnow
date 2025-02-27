#include "socket.hh"

using namespace std;

class RawSocket : public DatagramSocket
{
public:
  RawSocket() : DatagramSocket( AF_INET, SOCK_RAW, IPPROTO_RAW ) {}
};

int main()
{
  // construct an Internet or user datagram here, and send using the RawSocket as in the Jan. 10 lecture
  RawSocket sock;
  string host;
  sock.connect(Address(host, "http"));
  std::string loca = "This is my message!";
  sock.write(loca);
  sock.close();

  return 0;
}