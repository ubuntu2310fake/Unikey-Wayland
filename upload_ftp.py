#!/usr/bin/env python3
import ftplib
import os
import sys
import time

def upload_files_single_session(ftp_server, ftp_path, file_paths):
    print(f" -> Python FTP: Khởi tạo kết nối duy nhất tới {ftp_server}...")
    max_retries = 3
    for attempt in range(max_retries):
        try:
            ftp = ftplib.FTP()
            ftp.connect(ftp_server, 21, timeout=60)
            ftp.login() # anonymous login
            
            # Chuyển thư mục step-by-step
            for folder in ftp_path.split('/'):
                if folder:
                    ftp.cwd(folder)
            
            # Upload từng file trong cùng một phiên kết nối
            for file_path in file_paths:
                if not os.path.exists(file_path):
                    print(f" -> [BỎ QUA] File không tồn tại: {file_path}")
                    continue
                basename = os.path.basename(file_path)
                print(f" -> Python FTP: Đang upload {basename}...")
                with open(file_path, 'rb') as f:
                    ftp.storbinary(f"STOR {basename}", f)
                print(f" -> Python FTP: Upload thành công {basename}!")
                
            ftp.quit()
            print(" -> Python FTP: Hoàn tất phiên kết nối và đóng cổng thành công!")
            return True
        except Exception as e:
            print(f" -> Python FTP Error (lượt thử {attempt + 1}/{max_retries}): {e}")
            if attempt < max_retries - 1:
                time.sleep(5)
            else:
                raise e

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: upload_ftp.py <server> <path> <file1> [file2] [file3] ...")
        sys.exit(1)
    
    server = sys.argv[1]
    path = sys.argv[2]
    files_to_upload = sys.argv[3:]
    
    try:
        upload_files_single_session(server, path, files_to_upload)
    except Exception as e:
        sys.exit(1)
