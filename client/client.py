import socket
import os
import re

# 全局变量，Buffer Size
BUFFER_SIZE = 8192

# 读取多行响应，返回最后一行的状态码 code
def read_response(sock):
    while True:
        data = sock.recv(BUFFER_SIZE)
        if not data:
            return None
        else:
            data_str = data.decode()
            lines = data_str.split('\r\n')
            for line in lines[:-1]:
                print(line)
                code = line[:3]
                if line[3] == ' ': # the end line of response
                    return int(code)

def connect_to_ftp_server(server, port=21):
    try:
        control_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        control_sock.connect((server, port))
        read_response(control_sock)
        return control_sock
    except Exception as e:
        print(f"Error connecting to FTP server: {str(e)}")
        return None


def set_up_data_sock(mode, port_listen_sock, pasv_data_sock):
    data_sock = None
    if mode == 'a':
        data_sock, addr = port_listen_sock.accept()
        port_listen_sock.close()
    elif mode == 'p':
        data_sock = pasv_data_sock
    return data_sock


## 原生FTP命令

def USER(control_sock, username):
    control_sock.send(f"USER {username}\r\n".encode())
    read_response(control_sock)

def PASS(control_sock, password):
    control_sock.send(f"PASS {password}\r\n".encode())
    read_response(control_sock)

def PORT(control_sock):
    port = 0
    listen_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listen_sock.bind(('', port))    # 绑定到随机端口
    listen_sock.listen(1)
    port = listen_sock.getsockname()[1]
    ip = control_sock.getsockname()[0]
    ip_parts = ip.split('.')
    port_parts = [str(port // 256), str(port % 256)]
    port_param = ','.join(ip_parts + port_parts)
    control_sock.send(f"PORT {port_param}\r\n".encode())
    code = read_response(control_sock)
    if code == 200:
        return listen_sock
    else:
        listen_sock.close()
        return None

def PASV(control_sock):
    control_sock.send("PASV\r\n".encode())
    response = control_sock.recv(BUFFER_SIZE)
    print(response.decode())
    if response.startswith(b"227"):
        pattern = r'(\d+,){5}\d+'
        result = re.search(pattern, response.decode())
        if result:
            addr = result.group()
            addr_parts = addr.split(',')
            ip = '.'.join(addr_parts[:4])
            port = (int(addr_parts[4]) * 256) + int(addr_parts[5])
            data_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            data_sock.connect((ip, port))
            return data_sock
        else:
            return None
    else:
        return None

def LIST(control_sock, path, mode, port_listen_sock, pasv_data_sock):
    control_sock.send(f"LIST {path}\r\n".encode())
    code = read_response(control_sock)
    if code == 150:
        data_sock = set_up_data_sock(mode, port_listen_sock, pasv_data_sock)
        while True:
            data = data_sock.recv(BUFFER_SIZE)
            if data:
                print(data.decode())
            else:
                break
        data_sock.close()
        read_response(control_sock)

def RETR(control_sock, remote, mode, port_listen_sock, pasv_data_sock, rest_pos=0):
    local = os.path.basename(remote)
    open_mode = 'wb'
    if rest_pos > 0:
        open_mode = 'ab'
    try:
        control_sock.send(f"RETR {remote}\r\n".encode())
        code = read_response(control_sock)
        if code == 150:
            data_sock = set_up_data_sock(mode, port_listen_sock, pasv_data_sock)
            with open(local, open_mode) as f:
                while True:
                    data = data_sock.recv(BUFFER_SIZE)
                    if data:
                        f.write(data)
                    else:
                        break
            data_sock.close()
            read_response(control_sock)
    except FileNotFoundError as e:
        print(f"Local path error: {str(e)}")
    except Exception as e:
        print(f"Error sending RETR command: {str(e)}")


def STOR(control_sock, local, mode, port_listen_sock, pasv_data_sock, rest_pos=0):
    if not os.path.isfile(local):
        print(f"Local file <{local}> does not exist")
        return
    try:
        control_sock.send(f"STOR {local}\r\n".encode())
        code = read_response(control_sock)
        if code == 150:
            data_sock = set_up_data_sock(mode, port_listen_sock, pasv_data_sock)
            with open(local, 'rb') as f:
                if rest_pos > 0:
                    f.seek(rest_pos)
                while True:
                    data = f.read(BUFFER_SIZE)
                    if data:
                        data_sock.send(data)
                    else:
                        break
            data_sock.close()
            read_response(control_sock)
    except Exception as e:
        print(f"Error sending STOR command: {str(e)}")

def PWD(control_sock):
    control_sock.send("PWD\r\n".encode())
    read_response(control_sock)

def CWD(control_sock, path):
    control_sock.send(f"CWD {path}\r\n".encode())
    read_response(control_sock)

def MKD(control_sock, path):
    control_sock.send(f"MKD {path}\r\n".encode())
    read_response(control_sock)

def RMD(control_sock, path):
    control_sock.send(f"RMD {path}\r\n".encode())
    read_response(control_sock)

def RNFR(control_sock, path):
    control_sock.send(f"RNFR {path}\r\n".encode())
    read_response(control_sock)

def RNTO(control_sock, path):
    control_sock.send(f"RNTO {path}\r\n".encode())
    read_response(control_sock)

def SYST(control_sock):
    control_sock.send("SYST\r\n".encode())
    read_response(control_sock)

def REST(control_sock, pos):
    control_sock.send(f"REST {pos}\r\n".encode())
    code = read_response(control_sock)
    if code == 350:
        return int(pos)
    return 0

def TYPE(control_sock, type):
    control_sock.send(f"TYPE {type}\r\n".encode())
    read_response(control_sock)

def QUIT(control_sock):
    control_sock.send("QUIT\r\n".encode())
    read_response(control_sock)
    control_sock.close()


if __name__ == "__main__":
    control_sock = None # 控制连接套接字
    connected = False   # 是否已连接到服务器
    pasv_data_sock = None    # 数据传输套接字
    port_listen_sock = None  # PORT 指定的监听套接字
    mode = None         # 数据传输模式
    rest_pos = 0        # REST 指定的位置
    last_verb = None    # 上一条命令
    verb_list = ['OPEN', 'USER', 'PASS', 'PWD', 'CWD', 'PORT', 'PASV',
                  'LIST', 'RETR', 'STOR', 'MKD', 'RMD', 'RNFR', 'RNTO',
                    'SYST', 'REST', 'TYPE', 'QUIT', 'HELP']
    print("Welcome to FTP client!")
    print("Available commands: ")
    print(' '.join(verb_list))
    while True:
        print("ftp> ", end='')
        command = input()
        verb = command.split(' ')[0]
        if verb not in verb_list:
            print("Unknown command")
            continue
        if verb == 'HELP':
            print("Available commands: ")
            print(' '.join(verb_list))
            continue
        if verb != 'OPEN' and not connected:
            print("Please connect to server first: OPEN")
            continue
        # print(f"last_verb: {last_verb}", f"verb: {verb}")
        if (verb == 'RETR' or verb == 'STOR') and last_verb != 'REST':
            print("REST is set to 0")
            rest_pos = 0
        last_verb = verb
        if verb == 'OPEN':
            if len(command.split(' ')) == 1 or len(command.split(' ')) > 3:
                print("usage: OPEN host [port]")
                continue
            ip = command.split(' ')[1]
            port = 21
            if len(command.split(' ')) == 3:
                port = int(command.split(' ')[2])
            control_sock = connect_to_ftp_server(ip, port)
            if control_sock:
                connected = True
        elif verb == 'USER':
            if len(command.split(' ')) == 1 or len(command.split(' ')) > 2:
                print("usage: USER username")
                continue
            username = command.split(' ')[1]
            USER(control_sock, username)
        elif verb == 'PASS':
            if len(command.split(' ')) == 1 or len(command.split(' ')) > 2:
                print("usage: PASS password")
                continue
            password = command.split(' ')[1]
            PASS(control_sock, password)
        elif verb == 'PWD':
            if len(command.split(' ')) != 1:
                print("usage: PWD")
                continue
            PWD(control_sock)
        elif verb == 'CWD':
            if len(command.split(' ')) != 2:
                print("usage: CWD path")
                continue
            path = command.split(' ')[1]
            CWD(control_sock, path)
        elif verb == 'PORT':
            if len(command.split(' ')) != 1:
                print("usage: PORT")
                continue
            port_listen_sock = PORT(control_sock)
            mode = 'a'
        elif verb == 'PASV':
            if len(command.split(' ')) != 1:
                print("usage: PASV")
                continue
            pasv_data_sock = PASV(control_sock)
            mode = 'p'
        elif verb == 'LIST':
            if len(command.split(' ')) > 2:
                print("usage: LIST [path]")
                continue
            path = ''
            if len(command.split(' ')) == 2:
                path = command.split(' ')[1]
            LIST(control_sock, path, mode, port_listen_sock, pasv_data_sock)
        elif verb == 'RETR':
            if len(command.split(' ')) != 2:
                print("usage: RETR remote")
                continue
            remote = command.split(' ')[1]
            RETR(control_sock, remote, mode, port_listen_sock, pasv_data_sock, rest_pos)
        elif verb == 'STOR':
            if len(command.split(' ')) != 2:
                print("usage: STOR local")
                continue
            local = command.split(' ')[1]
            STOR(control_sock, local, mode, port_listen_sock, pasv_data_sock, rest_pos)
        elif verb == 'MKD':
            if len(command.split(' ')) != 2:
                print("usage: MKD path")
                continue
            path = command.split(' ')[1]
            MKD(control_sock, path)
        elif verb == 'RMD':
            if len(command.split(' ')) != 2:
                print("usage: RMD path")
                continue
            path = command.split(' ')[1]
            RMD(control_sock, path)
        elif verb == 'RNFR':
            if len(command.split(' ')) != 2:
                print("usage: RNFR path")
                continue
            path = command.split(' ')[1]
            RNFR(control_sock, path)
        elif verb == 'RNTO':
            if len(command.split(' ')) != 2:
                print("usage: RNTO path")
                continue
            path = command.split(' ')[1]
            RNTO(control_sock, path)
        elif verb == 'SYST':
            if len(command.split(' ')) != 1:
                print("usage: SYST")
                continue
            SYST(control_sock)
        elif verb == 'REST':
            if len(command.split(' ')) != 2:
                print("usage: REST pos")
                continue
            pos = command.split(' ')[1]
            rest_pos = REST(control_sock, pos)
        elif verb == 'TYPE':
            if len(command.split(' ')) != 2:
                print("usage: TYPE ( A | I )")
                continue
            type = command.split(' ')[1]
            if type not in ['A', 'I']:
                print("usage: TYPE ( A | I )")
                continue
            TYPE(control_sock, type)
        elif verb == 'QUIT':
            if len(command.split(' ')) != 1:
                print("usage: QUIT")
                continue
            QUIT(control_sock)
            connected = False