import sys
sys.path.append('gen-py')

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TCompactProtocol

from ping import Ping


def ping():
    transport = TSocket.TSocket('127.0.0.1', 9090)
    tranport = TTransport.TFramedTransport(transport)
    protocol = TCompactProtocol.TCompactProtocol(tranport)
    client = Ping.Client(protocol)
    tranport.open()
    client.ping()
    tranport.close()


def main():
    ping()


if __name__ == '__main__':
    main()
