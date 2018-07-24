import boto3
import shutil
import os
import datetime
import time


# TODO TEST THIS!

# THIS ASSUMES THAT websocket_stuff/full EXISTS! MAKE SURE IT DOES OR STUFF WILL GO WEIRD!

# Basically ripped from
# https://www.developerfiles.com/upload-files-to-s3-with-python-keeping-the-original-folder-structure/
def transfer_folder_to_bucket(folder_name, bucket_name):
    s3 = boto3.resource("s3")
    bucket = s3.Bucket(bucket_name)
    for root, dirs, files in os.walk(folder_name):  # I'm honestly not entirely sure how os.walk works, sorry guys
        for file in files:
            full_path = os.path.join(root, file)
            with open(full_path, 'rb') as dat:
                print(f"uploading {'full' + full_path[len(folder_name):]}")
                bucket.put_object(Key="full" + full_path[len(folder_name):], Body=dat)


def main():
    os.chdir("full")
    FULL_PATH = os.getcwd()
    pairs = [file for file in os.listdir() if os.path.isdir(file)]
    print(pairs)
    while True:
        for pair in pairs:
            os.chdir(os.path.join(FULL_PATH, pair) )
            date_dirs = [obj for obj in os.listdir() if os.path.isdir(obj)]
            now_day = datetime.datetime.utcnow().day
            for folder in date_dirs:
                print(folder)
                if now_day - int(folder[-2:]) >= 0:  # if the folder is from yesterday or farther back
                    transfer_folder_to_bucket(folder, "cryptoorderbookdata")
                    shutil.rmtree(folder)
        time.sleep(3600)  # wait an hour before polling again



if __name__ == "__main__":
    main()