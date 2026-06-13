# check_public_s3.py
# Audits every S3 bucket for Block Public Access (BPA).
#   protected      = all 4 BPA flags True
#   At risk        = config exists but some flag False
#   not restricted = no BPA config at all (caught via ClientError)
# boto3 gotchas banked:
#   - methods need the client prefix: s3.get_public_access_block(...)
#   - args are keyword: Bucket=name, not positional
#   - specific AWS errors aren't importable classes -> catch ClientError,
#     branch on e.response['Error']['Code']
import boto3
from botocore.exceptions import ClientError
s3=boto3.client('s3')
response=s3.list_buckets()
for check in response['Buckets']:
    try:
        response2=s3.get_public_access_block(Bucket=check['Name'])
        check2=response2['PublicAccessBlockConfiguration']
        if check2['BlockPublicAcls'] and check2['IgnorePublicAcls'] and check2['BlockPublicPolicy'] and check2['RestrictPublicBuckets']:
            print("protected")
        else:
            print("At risk")
    except ClientError as e:
        if e.response['Error']['Code'] == 'NoSuchPublicAccessBlockConfiguration':
            print("error,not restricted")
        else:
            print("Couldnt check")