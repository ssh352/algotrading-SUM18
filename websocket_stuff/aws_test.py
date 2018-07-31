import boto3
import lzma
import csv
import shutil

def main():
    print("Uploading...")
    shutil.make_archive("test compression", 'gztar', "full")
    print("Upload done!")

if __name__ == "__main__":
    main()