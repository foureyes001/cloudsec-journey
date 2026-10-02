import boto3
from botocore.exceptions import ClientError

PREFIX = "collegeproj-data-"
REGION = "us-east-1"


def main():
    s3 = boto3.client("s3", region_name=REGION)

    matching_buckets = sorted(
        bucket["Name"]
        for bucket in s3.list_buckets().get("Buckets", [])
        if bucket["Name"].startswith(PREFIX)
    )

    no_policy = []

    for bucket_name in matching_buckets:
        try:
            s3.get_bucket_policy(Bucket=bucket_name)
            status = "HAS POLICY"
        except ClientError as e:
            code = e.response.get("Error", {}).get("Code")
            if code == "NoSuchBucketPolicy":
                status = "NO POLICY"
                no_policy.append(bucket_name)
            else:
                status = f"POLICY STATUS UNKNOWN ({code})"

        print(f"{bucket_name}: {status}")

    print("\nBuckets without a bucket policy:")
    for bucket_name in no_policy:
        print(bucket_name)


if __name__ == "__main__":
    main()
