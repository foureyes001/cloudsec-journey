import boto3
from botocore.exceptions import ClientError


def check_bucket_policies():
    s3_client = boto3.client("s3", region_name="us-east-1")

    prefix = "collegeproj-data-"
    matching_buckets = []

    # Retrieve all buckets
    try:
        response = s3_client.list_buckets()
        all_buckets = response.get("Buckets", [])
        matching_buckets = [
            b["Name"] for b in all_buckets if b["Name"].startswith(prefix)
        ]
    except ClientError as e:
        print(f"Error listing buckets: {e.response.get('Error', {}).get('Message', str(e))}")
        return

    if not matching_buckets:
        print(f"No buckets found matching prefix '{prefix}'.")
        return

    buckets_without_policy = []

    print(f"Found {len(matching_buckets)} bucket(s) matching '{prefix}':\n")

    for bucket_name in matching_buckets:
        try:
            s3_client.get_bucket_policy(Bucket=bucket_name)
            print(f"Bucket: {bucket_name} | Policy: Present")
        except ClientError as e:
            error_code = e.response.get("Error", {}).get("Code")
            if error_code == "NoSuchBucketPolicy":
                print(f"Bucket: {bucket_name} | Policy: None")
                buckets_without_policy.append(bucket_name)
            else:
                error_msg = e.response.get("Error", {}).get("Message", str(e))
                print(f"Bucket: {bucket_name} | Error: {error_code} - {error_msg}")

    print("\n" + "=" * 50)
    print("Buckets without a bucket policy:")
    print("=" * 50)
    if buckets_without_policy:
        for name in buckets_without_policy:
            print(f"- {name}")
    else:
        print("None (all matching buckets have a bucket policy).")


if __name__ == "__main__":
    check_bucket_policies()
