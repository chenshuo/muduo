import sys
sys.path.append('gen-py')

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from echo import Echo


def echo(s):
    transport = TSocket.TSocket('127.0.0.1', 9090)
    tranport = TTransport.TFramedTransport(transport)
    protocol = TBinaryProtocol.TBinaryProtocol(tranport)
    client = Echo.Client(protocol)
    tranport.open()
    s = client.echo(s)
    tranport.close()

    return s


def main():
    print(echo('42'))


if __name__ == '__main__':
    main()
