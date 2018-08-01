import boto3
import shutil
import os
import datetime
import time


# TODO TEST THIS!

# THIS ASSUMES THAT websocket_stuff/full EXISTS! MAKE SURE IT DOES OR STUFF WILL GO WEIRD!

# Basically ripped from
# https://www.developerfiles.com/upload-files-to-s3-with-python-keeping-the-original-folder-structure/
def transfer_folder_to_bucket(folder_name, bucket):
    for root, dirs, files in os.walk(folder_name):  # I'm honestly not entirely sure how os.walk works, sorry guys
        for file in files:
            if not file.endswith(".DS_STORE"):
                full_path = os.path.join(root, file)
                with open(full_path, 'rb') as data:
                    print(f"uploading {'full/' + folder_name + full_path[len(folder_name):]}")
                    bucket.put_object(Key="full/" + folder_name + full_path[len(folder_name):], Body=data)
                    print("removing")
                    os.remove(full_path)


def main():
    os.chdir("full")
    s3 = boto3.resource("s3")  # Create s3 resource to interface with S3
    bucket = s3.Bucket("cryptoorderbookdata")  # Create bucket object to store files in
    while True:
        file_list = [file for file in os.listdir()]  # List of files in the 'full' folder
        print(file_list)
        for file in file_list:
            if os.path.isdir(file):  # If file is a BCH-USD folder or the like
                os.chdir(file)
                date_dirs = [obj for obj in os.listdir() if os.path.isdir(obj)]
                now_day = datetime.datetime.utcnow().day
                for folder in date_dirs:
                    if now_day - int(folder[-2:]) >= 1:
                        print(folder)
                        print("compressing" + folder)
                        shutil.make_archive(folder, "gztar", folder)
                        print("removing" + folder)
                        shutil.rmtree(folder)
                os.chdir("..")
                print("uploading" + file)
                transfer_folder_to_bucket(file, bucket)
            elif file.endswith(".log"):
                data = open(file, 'rb')
                bucket.put_object(Key=file, Body=data)
                os.remove(file)
        time.sleep(3600 * 4)  # wait four hours before polling again


if __name__ == "__main__":
    main()