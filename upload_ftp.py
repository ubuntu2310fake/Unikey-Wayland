#!/usr/bin/env python3
import ftplib
import os
import sys
import time

def upload_file(ftp_server, ftp_path, file_path):
    print(f" -> Python FTP: Uploading {os.path.basename(file_path)}...")
    max_retries = 3
    for attempt in range(max_retries):
        try:
            ftp = ftplib.FTP()
            # Connect to server
            ftp.connect(ftp_server, 21, timeout=30)
            ftp.login() # anonymous login
            
            # Change directory to destination
            # Launchpad FTP might require navigating step-by-step
            for folder in ftp_path.split('/'):
                if folder:
                    ftp.cwd(folder)
            
            # Send file in binary mode
            with open(file_path, 'rb') as f:
                ftp.storbinary(f"STOR {os.path.basename(file_path)}", f)
            
            ftp.quit()
            print(f" -> Python FTP: Uploaded {os.path.basename(file_path)} successfully!")
            return True
        except Exception as e:
            print(f" -> Python FTP Error (attempt {attempt + 1}/{max_retries}): {e}")
            if attempt < max_retries - 1:
                time.sleep(5)
            else:
                raise e

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: upload_ftp.py <server> <path> <file_to_upload>")
        sys.exit(1)
    
    server = sys.argv[1]
    path = sys.argv[2]
    file_to_upload = sys.argv[3]
    
    if not os.path.exists(file_to_upload):
        print(f"File not found: {file_to_upload}")
        sys.exit(1)
        
    try:
        upload_file(server, path, file_to_upload)
    except Exception as e:
        sys.exit(1)
