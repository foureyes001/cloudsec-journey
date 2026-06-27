import boto3
import json

dynamodb=boto3.resource('dynamodb')
table= dynamodb.Table('cloud-resume-visitor-count')

def lambda_handler(event,context):
    response = table.update_item(
    Key={
            'id':'visitors'},
    UpdateExpression='ADD #c :inc',
    ExpressionAttributeNames={
        '#c': 'count'},
    ExpressionAttributeValues={
        ':inc':1},
    ReturnValues='UPDATED_NEW' )
    new_count=int(response['Attributes']['count'])    
    return {
    'statusCode': 200,
    'headers': { 'Access-Control-Allow-Origin': '*' },   
    'body': json.dumps({ 'count': new_count })            
    }
