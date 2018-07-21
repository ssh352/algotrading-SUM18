import boto3

def main():
    print("Uploading...")
    s3 = boto3.resource('s3')
    data = open('README.md', 'rb')
    s3.Bucket('cryptoorderbookdata').put_object(Key='README.md', Body=data)
    print("Upload done!")

if __name__ == "__main__":
    main()