import boto3

# Create IAM client
iam = boto3.client('iam')

# List users with the pagination interface
paginator = iam.get_paginator('list_users')
for response in paginator.paginate():
    for user in response['Users']:
        name=user['UserName']
        print(name)
        resp = iam.list_mfa_devices(UserName=name)
        if not resp['MFADevices']:
            print(f"{name} ->NO MFA")
        else: 
            print(f"{name} ->MFA OK")