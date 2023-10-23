import sys
import re
import os
import socket
import threading
from PySide6.QtWidgets import QApplication, QMainWindow, QMessageBox, QTableWidgetItem, QTableWidget, QMenu, QInputDialog, QFileDialog
from PySide6.QtGui import QAction
from PySide6.QtCore import Signal, Slot, QObject, QThread, Qt
from ui_mainwindow import Ui_MainWindow


# 用于在下载和上传子线程中更新进度条, 线程中不能直接更新UI, 必须通过信号槽机制
class Worker(QObject):
    finished = Signal(int)             # 传输完成信号
    progress_changed = Signal(int)     # 进度条信号
    data_sock: socket.socket
    filename: str  # local filename or remote filename
    filesize: int
    rest_pos: int
    mode: str   # download or upload
    close_event: threading.Event    # 用于停止传输子线程
    def __init__(self, close_event:threading.Event):
        super().__init__()
        self.rest_pos = 0
        self.data_sock = None
        self.filename = None
        self.filesize = 0
        self.mode = None
        self.close_event = close_event

    def set_info(self, mode:str, rest_pos:int, data_sock:socket.socket, filename:str, filesize:int):
        self.mode = mode
        self.rest_pos = rest_pos
        self.data_sock = data_sock
        self.filename = filename
        self.filesize = filesize
    
    def run(self):
        if self.mode == 'download':
            self.download()
        elif self.mode == 'upload':
            self.upload()
        self.mode = None

    def download(self):
        if self.rest_pos > 0:
            open_mode = 'ab'
        else:
            open_mode = 'wb'
        transfered_size = self.rest_pos
        local_file = os.path.basename(self.filename)
        try:
            with open(local_file, open_mode) as f:
                while True:
                    if self.close_event.is_set():   # stopBtn点击后，设置event，停止传输
                        break
                    data = self.data_sock.recv(8192)
                    if data:
                        f.write(data)
                        transfered_size += len(data)
                        progress = transfered_size / self.filesize * 100
                        self.progress_changed.emit(progress)
                    else:
                        break
            self.data_sock.close()
            self.finished.emit(transfered_size)
        except:
            self.data_sock.close()
            self.finished.emit(transfered_size)

    def upload(self):
        transfered_size = self.rest_pos
        try:
            with open(self.filename, "rb") as f:
                f.seek(self.rest_pos)
                while True:
                    if self.close_event.is_set():   # stopBtn点击后，设置event，停止传输
                        break
                    data = f.read(8192)
                    if data:
                        self.data_sock.send(data)
                        transfered_size += len(data)
                        progress = transfered_size / self.filesize * 100
                        self.progress_changed.emit(progress)
                    else:
                        break
                self.data_sock.close()
                self.finished.emit(transfered_size)
        except:
            self.data_sock.close()
            self.finished.emit(transfered_size)

class MainWindow(QMainWindow):
    ip: str
    port: str
    username: str
    password: str
    remote_path: str
    control_sock: socket.socket
    data_sock: socket.socket
    rest_pos: int
    connected: bool
    curTransFile: dict

    menu: QMenu
    delete_action: QAction
    create_directory_action: QAction
    download_action: QAction
    upload_action: QAction
    rename_action: QAction

    worker: Worker
    worker_thread: QThread
    close_event: threading.Event    # 用于关闭子线程
    is_stopped: bool    # 用于判断是否停止传输

    def __init__(self):
        super(MainWindow, self).__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        # 166.111.83.113
        # self.ip = '166.111.83.113'
        # self.port = '21'
        # self.username = 'thss'
        # self.password = 'thss2023'

        self.remote_path = '/'
        self.control_sock = None
        self.data_sock = None
        self.rest_pos = 0
        self.connected = False

        self.curTransFile = {
            "mode": None,   # download or upload
            "local_path": None,
            "remote_path": None,
            "filesize": 0,
            "transfered_size": 0
        }

        # 右键菜单 - file list
        self.menu = None    
        self.delete_action = None
        self.create_directory_action = None
        self.download_action = None
        self.upload_action = None
        self.rename_action = None

        self.close_event = threading.Event()
        self.is_stopped = True
        # progress bar
        self.worker = Worker(self.close_event)
        self.worker_thread = QThread()
        self.worker.moveToThread(self.worker_thread)
        self.worker_thread.started.connect(self.worker.run)
        self.worker.progress_changed.connect(self.ui.progressBar.setValue)
        # self.worker.finished.connect(self.read_response) # 读取最后一行响应
        # self.worker.finished.connect(self.worker_thread.quit)
        # self.worker.finished.connect(self.ls)
        self.worker.finished.connect(self.on_worker_finished)


        self.init_table_setting()
        self.init_menu()
        self.signal_slot()

    # signal and slot
    def signal_slot(self):
        self.ui.connBtn.clicked.connect(self.on_connBtn_clicked)
        self.ui.disConnBtn.clicked.connect(self.disconnect)
        self.ui.lineEditRemote.returnPressed.connect(self.on_lineEditRemote_returnPressed)
        self.ui.stopBtn.clicked.connect(self.on_stopBtn_clicked)
        self.ui.restartBtn.clicked.connect(self.on_restartBtn_clicked)
    

    # @Slot()
    def on_connBtn_clicked(self):
        self.ip = self.ui.lineEditHost.text()
        self.port = self.ui.lineEditPort.text()
        self.username = self.ui.lineEditUser.text()
        self.password = self.ui.lineEditPass.text()
        if self.port == "":
            self.port = str(21)
        # 判断port是否为数字
        if self.port.isdigit() == False:
            QMessageBox.information(self, "Warning", "Please enter the correct port.")
            return
        if self.ip == "" or self.username == "" or self.password == "":
            self.ui.statusbar.showMessage("Please enter the information.")
            # 弹出对话框
            QMessageBox.information(self, "Warning", "Please enter the information.")
        else:
            self.ui.textEdit.append("status: Connecting to the server...")
            self.connect()
        self.ui.textEdit.append("状态:    Connecting to the server...")
        self.connect()

    def on_lineEditRemote_returnPressed(self):
        path = self.ui.lineEditRemote.text()
        self.cwd(path)

    def on_stopBtn_clicked(self):
        if self.curTransFile["transfered_size"] == self.curTransFile["filesize"]:
            return
        if self.is_stopped:
            QMessageBox.information(self, "Warning", "传输已经停止")
            return
        # self.close_sock(self.data_sock)   # 这种方法会有问题，可能导致死锁
        # # 竞争资源：子线程可能正在使用data_sock, 所以通过设置event来关闭子线程
        self.close_event.set()
    
    def on_restartBtn_clicked(self):
        if self.curTransFile["transfered_size"] == self.curTransFile["filesize"]:
            return
        if not self.is_stopped:
            QMessageBox.information(self, "Warning", "正在传输过程中")
            return
        
        pos = self.curTransFile["transfered_size"]
        self.pasv()
        if not self.data_sock:
            return
        self.control_sock.send(f"REST {pos}\r\n".encode())
        code = self.read_response()
        if code == 350:
            self.ui.textEdit.append(f"状态:    重传位置为{pos}.")
        else:
            self.ui.textEdit.append(f"状态:    不支持REST")
            pos = 0
        if self.curTransFile["mode"] == "download":
            self.control_sock.send(f"RETR {self.curTransFile['remote_path']}\r\n".encode())
            code = self.read_response()
            if code != 150:
                self.ui.textEdit.append(f"状态:    重传'{self.curTransFile['remote_path']}'失败.")
                self.close_sock(self.data_sock)
                return
            self.close_event.clear()
            self.is_stopped = False
            self.worker.set_info("download", pos, self.data_sock, self.curTransFile["remote_path"], self.curTransFile["filesize"])
            self.worker_thread.start()
        elif self.curTransFile["mode"] == "upload":
            basename = os.path.basename(self.curTransFile["local_path"])
            self.control_sock.send(f"STOR {basename}\r\n".encode())
            code = self.read_response()
            if code != 150:
                self.ui.textEdit.append(f"状态:    重传'{self.curTransFile['local_path']}'失败.")
                self.close_sock(self.data_sock)
                return
            self.close_event.clear()    # 重传时，需要清除event
            self.is_stopped = False    # 重传时，需要设置is_stopped 
            self.worker.set_info("upload", pos, self.data_sock, self.curTransFile["local_path"], self.curTransFile["filesize"])
            self.worker_thread.start()

    def on_worker_finished(self, transfered_size):
        self.ui.textEdit.append(f"状态:    {transfered_size} bytes transfered.")
        self.is_stopped = True
        self.read_response()
        self.worker_thread.quit()
        self.ls()
        self.curTransFile["transfered_size"] = transfered_size
        
    
    def set_curTransFile(self, mode, local_path, remote_path, filesize, transfered_size):
        self.curTransFile["mode"] = mode
        self.curTransFile["local_path"] = local_path
        self.curTransFile["remote_path"] = remote_path
        self.curTransFile["filesize"] = filesize
        self.curTransFile["transfered_size"] = transfered_size

    def init_table_setting(self):
        ## 文件列表
        self.ui.tableWidgetRemote.setColumnCount(4)
        self.ui.tableWidgetRemote.setHorizontalHeaderLabels(["文件名", "文件类型", "文件大小", "上次修改时间"])
        self.ui.tableWidgetRemote.setSelectionBehavior(QTableWidget.SelectRows)  # 设置为选中整行
        self.ui.tableWidgetRemote.horizontalHeader().setDefaultAlignment(Qt.AlignLeft)   # 设置header左对齐
        self.ui.tableWidgetRemote.setSortingEnabled(True)  # 设置header点击排序
        self.ui.tableWidgetRemote.horizontalHeader().setHighlightSections(False)  # 设置header不加粗
        self.ui.tableWidgetRemote.setShowGrid(False)  # 设置item不显示分割线
        self.ui.tableWidgetRemote.setEditTriggers(QTableWidget.NoEditTriggers)  # 设置item不可编辑
        # 鼠标右键菜单
        self.ui.tableWidgetRemote.setContextMenuPolicy(Qt.CustomContextMenu)
        self.ui.tableWidgetRemote.customContextMenuRequested.connect(self.show_context_menu)
        

    def show_context_menu(self, position):
        if not self.connected:
            return
        self.menu.exec(self.ui.tableWidgetRemote.viewport().mapToGlobal(position))


    def set_table_content(self, files):
        self.ui.tableWidgetRemote.clearContents()
        self.ui.tableWidgetRemote.setRowCount(len(files))
        for row, file in enumerate(files):
            for col, value in enumerate(file):
                item = QTableWidgetItem(value)
                self.ui.tableWidgetRemote.setItem(row, col, item)
    
    def init_menu(self):
        self.menu = QMenu(self.ui.tableWidgetRemote)
        self.delete_action = QAction("删除")
        self.create_directory_action = QAction("创建目录")
        self.download_action = QAction("下载")
        self.upload_action = QAction("上传")
        self.rename_action = QAction("重命名")
        self.menu.addAction(self.delete_action)
        self.menu.addAction(self.create_directory_action)
        self.menu.addAction(self.download_action)
        self.menu.addAction(self.upload_action)
        self.menu.addAction(self.rename_action)
        self.delete_action.triggered.connect(self.delete_item)
        self.create_directory_action.triggered.connect(self.create_directory)
        self.download_action.triggered.connect(self.download)
        self.upload_action.triggered.connect(self.upload)
        self.rename_action.triggered.connect(self.rename)

    # 读取多行响应，返回最后一行的状态码 code
    def read_response(self):
        while True:
            data = self.control_sock.recv(1024)
            if not data:
                return None
            else:
                data_str = data.decode()
                lines = data_str.split('\r\n')
                for line in lines[:-1]:
                    code = line[:3]
                    if line[3] == ' ': # the end line of response
                        self.ui.textEdit.append("响应:    " + line)
                        return int(code)

    def close_sock(self, sock):
        if sock:
            sock.close()
            sock = None

    # 点击连接按钮后，连接到服务器，登录，获取当前目录，列出文件列表
    @Slot()
    def connect(self):
        try:
            self.control_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.control_sock.connect((self.ip, int(self.port)))
            self.read_response()
            self.ui.textEdit.append("状态:    Connected to the server.")
            self.ui.textEdit.append("状态:    Logging in...")
            self.control_sock.send(f"USER {self.username}\r\n".encode())
            self.read_response()
            self.control_sock.send(f"PASS {self.password}\r\n".encode())
            code = self.read_response()
            if code == 230:
                self.ui.textEdit.append("状态:    Logged in.")
                self.connected = True
                self.pwd()
                self.ui.lineEditRemote.setReadOnly(False)
                self.ls()
            else:
                self.ui.textEdit.append("状态:    Failed to log in.")
        except:
            self.ui.textEdit.append("状态:    Failed to connect to the server.")
    
    @Slot()
    def disconnect(self):
        self.control_sock.send("QUIT\r\n".encode())
        self.read_response()
        self.close_sock(self.control_sock)
        self.ui.textEdit.append("状态:    Disconnected from the server.")
        self.connected = False
        # clear the file list
        self.ui.lineEditRemote.setText("")
        self.ui.lineEditRemote.setReadOnly(True)
        self.ui.tableWidgetRemote.clearContents()
        self.ui.tableWidgetRemote.setRowCount(0)
        self.ui.curTransLabel.setText("传输文件:")
        self.ui.curTransLineEdit.setText("")
        self.ui.progressBar.setValue(0)
        self.set_curTransFile(None, None, None, 0, 0)

    def pasv(self):
        self.close_sock(self.data_sock)
        self.control_sock.send("PASV\r\n".encode())
        response = self.control_sock.recv(8192)
        self.ui.textEdit.append("响应:    " + response.decode().removesuffix('\r\n'))
        try:
            if response.startswith(b"227"):
                pattern = r'(\d+,){5}\d+'
                result = re.search(pattern, response.decode())
                if result:
                    addr = result.group()
                    addr_parts = addr.split(',')
                    ip = '.'.join(addr_parts[:4])
                    port = (int(addr_parts[4]) * 256) + int(addr_parts[5])
                    self.data_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.data_sock.connect((ip, port))
        except:
            self.ui.textEdit.append("状态:    Failed to establish data connection.")
            self.close_sock(self.data_sock)
    
    def ls(self):
        self.pasv()
        if not self.data_sock:
            return
        self.control_sock.send(f"LIST {self.remote_path}\r\n".encode())
        code = self.read_response()
        if code != 150:
            self.ui.textEdit.append(f"状态:    列出'{self.remote_path}'目录失败.")
            self.close_sock(self.data_sock)
            return
        data = self.data_sock.recv(8192)
        self.close_sock(self.data_sock)
        files = self.parse_list_response(data)

        # set table content
        self.set_table_content(files)
    
        code = self.read_response()
        if code == 226:
            self.ui.textEdit.append(f"状态:    列出'{self.remote_path}'目录成功.")
    
    def parse_list_response(self, data):
        # drwxr-xr-x 3 geh geh 4096 Oct 21 20:47 ftp
        lines = data.decode().split('\r\n')
        files = []
        for line in lines[:-1]:
            fileinfo = []   # name, type, size, modify
            parts = line.split(maxsplit=10) # 8
            fileinfo.append(parts[8])
            if parts[0].startswith('d'):
                fileinfo.append('DIR')
            else:
                fileinfo.append('FILE')
            fileinfo.append(parts[4])
            modify = ' '.join(parts[5:8])
            fileinfo.append(modify)
            files.append(fileinfo)
        return files


    def pwd(self):
        self.control_sock.send("PWD\r\n".encode())
        response = self.control_sock.recv(8192)
        response_str = response.decode()
        if response_str.startswith("257"):
            pattern = r'"(.*)"'
            result = re.search(pattern, response_str)
            if result:
                self.ui.lineEditRemote.setText(result.group(1))
                self.remote_path = result.group(1)
    
    def cwd(self, path):
        if path == self.remote_path:
            return
        self.control_sock.send(f"CWD {path}\r\n".encode())
        code = self.read_response()
        if code == 250:
            self.remote_path = path
            self.pwd()
            self.ls()
        else:
            self.ui.lineEditRemote.setText(self.remote_path)

    def download(self):
        if not self.is_stopped:
            QMessageBox.information(self, "Warning", "请先停止当前传输.")
            return
        selected_items = self.ui.tableWidgetRemote.selectedItems()
        if not selected_items:
            return
        filename = selected_items[0].text()
        filetype = selected_items[1].text()
        filesize = int(selected_items[2].text())
        if filetype == 'DIR':
            self.ui.textEdit.append(f"状态:    '{filename}'是目录，不能下载.")
            return
        self.ui.textEdit.append(f"状态:    下载'{filename}'...")
        self.pasv()
        if not self.data_sock:
            return
        self.control_sock.send(f"RETR {filename}\r\n".encode())
        code = self.read_response()
        if code != 150:
            self.ui.textEdit.append(f"状态:    下载'{filename}'失败.")
            self.close_sock(self.data_sock)
            return
        # thread = threading.Thread(target=self.download_thread, args=(filename, filesize))
        # thread.start()
        self.ui.curTransLabel.setText("下载远程文件:")
        remote_filename = self.remote_path
        if remote_filename == '/':
            remote_filename += filename
        else:
            remote_filename += '/' + filename
        self.ui.curTransLineEdit.setText(remote_filename)
        self.set_curTransFile("download", None, remote_filename, filesize, 0)

        self.close_event.clear()    # 清除event
        self.is_stopped = False     # 设置is_stopped
        self.worker.set_info("download", self.rest_pos, self.data_sock, filename, filesize)
        self.worker_thread.start()

    def download_thread(self, filename, filesize):
        if self.rest_pos > 0:
            open_mode = 'ab'
        else:
            open_mode = 'wb'
        transfered_size = 0
        try:
            with open(filename, open_mode) as f:
                while True:
                    data = self.data_sock.recv(8192)
                    if data:
                        f.write(data)
                        transfered_size += len(data)
                        progress = transfered_size / filesize * 100
                        print(f"{progress:.2f}%")
                        # progress bar
                        self.worker.progress_changed.emit(progress)
                    else:
                        break
            self.close_sock(self.data_sock)
            self.read_response()
        except:
            self.ui.textEdit.append(f"状态:    下载'{filename}'失败.")
            self.close_sock(self.data_sock)

    def upload(self):
        if not self.is_stopped:
            QMessageBox.information(self, "Warning", "请先停止当前传输.")
            return
        # 弹出对话框，选择文件
        filename = QFileDialog.getOpenFileName(self, "选择文件", "./")[0]
        basename = os.path.basename(filename)
        if not filename:
            return
        filesize = os.path.getsize(filename)
        self.ui.textEdit.append(f"状态:    上传'{filename}'...")
        self.pasv()
        if not self.data_sock:
            return
        self.control_sock.send(f"STOR {basename}\r\n".encode()) # 必须是basename
        code = self.read_response()
        if code != 150:
            self.ui.textEdit.append(f"状态:    上传'{filename}'失败.")
            self.close_sock(self.data_sock)
            return
        # thread = threading.Thread(target=self.upload_thread, args=(filename, filesize))
        # thread.start()
        self.ui.curTransLabel.setText("上传本地文件:")
        self.ui.curTransLineEdit.setText(filename)
        self.set_curTransFile("upload", filename, None, filesize, 0)

        self.close_event.clear()    # 清除event
        self.is_stopped = False     # 设置is_stopped
        self.worker.set_info("upload", self.rest_pos, self.data_sock, filename, filesize)
        self.worker_thread.start()

    def upload_thread(self, filename, filesize):
        transfered_size = self.rest_pos
        try:
            with open(filename, "rb") as f:
                f.seek(self.rest_pos)
                while True:
                    data = f.read(8192)
                    if data:
                        self.data_sock.send(data)
                        transfered_size += len(data)
                        progress = transfered_size / filesize * 100
                        print(f"{progress:.2f}%")
                    else:
                        break
                self.close_sock(self.data_sock)
                self.read_response()
                self.ui.textEdit.append(f"状态:    上传'{filename}'成功.")
                self.ls()
        except:
            self.ui.textEdit.append(f"状态:    上传'{filename}'失败.")
            self.close_sock(self.data_sock)
            return


    def rename(self):
        selected_items = self.ui.tableWidgetRemote.selectedItems()
        if not selected_items:
            return
        filename = selected_items[0].text()
        # 弹出对话框，输入新的文件名
        new_filename, ok = QInputDialog.getText(self, "重命名", "请输入新的文件名", text=filename)
        if not ok:
            return
        if new_filename == filename:
            return
        self.ui.textEdit.append(f"状态:    重命名'{filename}'为'{new_filename}'...")
        self.control_sock.send(f"RNFR {filename}\r\n".encode())
        self.read_response()
        self.control_sock.send(f"RNTO {new_filename}\r\n".encode())
        code = self.read_response()
        if code == 250:
            self.ui.textEdit.append(f"状态:    重命名'{filename}' -> '{new_filename}'成功.")
            self.ls()
        else:
            self.ui.textEdit.append(f"状态:    重命名'{filename}' -> '{new_filename}'失败.")
    
    def delete_item(self):
        selected_items = self.ui.tableWidgetRemote.selectedItems()
        if not selected_items:
            return
        filename = selected_items[0].text()
        filetype = selected_items[1].text()
        if filetype == 'DIR':
            self.ui.textEdit.append(f"状态:    删除目录'{filename}'...")
            self.control_sock.send(f"RMD {filename}\r\n".encode())
            code = self.read_response()
            if code == 250:
                self.ui.textEdit.append(f"状态:    删除目录'{filename}'成功.")
                self.ls()
            else:
                self.ui.textEdit.append(f"状态:    删除目录'{filename}'失败.")
        else:
            self.ui.textEdit.append(f"状态:    删除文件'{filename}'...")
            self.control_sock.send(f"DELE {filename}\r\n".encode())
            code = self.read_response()
            if code == 250:
                self.ui.textEdit.append(f"状态:    删除文件'{filename}'成功.")
                self.ls()
            else:
                self.ui.textEdit.append(f"状态:    删除文件'{filename}'失败.")


    def create_directory(self):
        # 弹出对话框，输入新的文件名
        new_filename, ok = QInputDialog.getText(self, "创建目录", "请输入新的目录名")
        if not ok:
            return
        self.ui.textEdit.append(f"状态:    创建目录'{new_filename}'...")
        self.control_sock.send(f"MKD {new_filename}\r\n".encode())
        code = self.read_response()
        if code == 257:
            self.ui.textEdit.append(f"状态:    创建目录'{new_filename}'成功.")
            self.ls()
        else:
            self.ui.textEdit.append(f"状态:    创建目录'{new_filename}'失败.")
    



if __name__ == "__main__":
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())