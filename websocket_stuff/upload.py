import boto3
import shutil
import os
import datetime
import time
from dateutil.parser import *


# TODO Generally tested, requires edge testing

# THIS ASSUMES THAT websocket_stuff/full EXISTS! MAKE SURE IT DOES OR STUFF WILL GO WEIRD!
# https://www.developerfiles.com/upload-files-to-s3-with-python-keeping-the-original-folder-structure/
# Not used of now
def transfer_folder_to_bucket(folder_name, bucket):
    for root, dirs, files in os.walk(folder_name):
        for file in files:
            if not file.endswith(".DS_STORE"):
                full_path = os.path.join(root, file)
                with open(full_path, 'rb') as data:
                    print(f"uploading {'full/' + folder_name + full_path[len(folder_name):]}")
                    bucket.put_object(Key="full/" + folder_name + full_path[len(folder_name):], Body=data)
                    os.remove(full_path)


def main():
    os.chdir("full")
    s3 = boto3.resource("s3")  # Create s3 resource to interface with S3
    bucket = s3.Bucket("cryptoorderbookdata")  # Create bucket object to store files in
    while True:
        # List of files in the 'full' folder, of form "XXX-USD" or "00000000.log"
        file_list = [file for file in os.listdir()]
        print(file_list)

        for file in file_list:
            # If file is a "XXX-USD" or similar directory
            if os.path.isdir(file):
                os.chdir(file)
                # List of date folders in XXX-USD folder
                date_dirs = [obj for obj in os.listdir() if os.path.isdir(obj)]
                print(date_dirs)
                now_date = datetime.datetime.utcnow().date()
                for date_dir in date_dirs:
                    # Convert folder name to date obj
                    folder_date = parse(date_dir).date()
                    # If folder is not from today
                    if (now_date - folder_date).days >= 1:
                        print("compressing" + date_dir)
                        shutil.make_archive(date_dir, "gztar", date_dir)
                        print("removing folder" + date_dir)
                        shutil.rmtree(date_dir)
                        print("uploading" + date_dir)
                        with open(date_dir + ".tar.gz", "rb") as data:
                            bucket.put_object(Key="full/" + file + date_dir + ".tar.gz", Body=data)
                        os.remove(date_dir + ".tar.gz")
                        print("removing" + date_dir)
                os.chdir("..")
            # Else if log file, upload
            elif file.endswith(".log"):
                with open(file, 'rb') as data:
                    bucket.put_object(Key=file, Body=data)
                os.remove(file)
        time.sleep(3600 * 4)  # Wait four hours before polling again


if __name__ == "__main__":
    main()