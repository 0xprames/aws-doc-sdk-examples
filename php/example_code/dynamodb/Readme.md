# Amazon DynamoDB code examples for the AWS SDK for PHP v3

## Purpose

Shows how to use the AWS SDK for PHP v3 to get started using operations in Amazon DynamoDB. Learn to create tables, add
items, update data, create custom queries, and delete tables and items.

*Amazon DynamoDB is a fully managed NoSQL database service that provides fast and predictable performance with seamless
scalability.*

## ⚠️ Important
* Running this code might result in charges to your AWS account.
* Running the tests might result in charges to your AWS account.
* We recommend that you grant your code least privilege. 
At most, grant only the minimum permissions required to perform the task. 
For more information, see 
[Grant least privilege](https://docs.aws.amazon.com/IAM/latest/UserGuide/best-practices.html#grant-least-privilege).
* This code is not tested in every AWS Region. 
For more information, see 
[AWS Regional Services](https://aws.amazon.com/about-aws/global-infrastructure/regional-product-services).

## Code examples

### Single actions

**Actions**

* [Creating a table](dynamodb_basics/GettingStartedWithDynamoDB.php)(`create_table`)
* [Deleting a table](DynamoDBService.php)(`delete_table`)
* [Deleting an item from a table](dynamodb_basics/GettingStartedWithDynamoDB.php)(`delete_item`)
* [Getting an item from a table](dynamodb_basics/GettingStartedWithDynamoDB.php)(`get_item`)
* [Putting an item into a table](dynamodb_basics/GettingStartedWithDynamoDB.php)(`put_item`)
* [Putting items loaded from a JSON file into a table](dynamodb_basics/GettingStartedWithDynamoDB.php)
  (`put_item`)
* [Querying items by using a key condition expression](dynamodb_basics/GettingStartedWithDynamoDB.php)
  (`query`)
* [Scanning a table for items](dynamodb_basics/GettingStartedWithDynamoDB.php)
  (`scan`)
* [Updating an item in a table](dynamodb_basics/GettingStartedWithDynamoDB.php)
  (`update_item`)

### Scenario examples

* [Getting started with DynamoDB](dynamodb_basics/GettingStartedWithDynamoDB.php)

## Running the examples
**Getting started with DynamoDB**

This scenario shows you how to create an Amazon DynamoDB table for storing movie data. The scenario loads movies into
the table from a JSON-formatted file, walks you through an interactive demo to add, update, and delete movies one at a
time, and shows you how to query for sets of movies.

```
composer install
```

Once your composer dependencies are successfully installed, you can run the interactive getting started file directly
with:

```
php Runner.php
```   

Or, to have the choices automatically selected, you can run it as part of a PHPUnit test with:

```
..\..\vendor\bin\phpunit DynamoDBBasicsTests.php
```

### Prerequisites

- You must have an AWS account, and have your default credentials and AWS Region configured as described in
the [AWS Tools and SDKs Shared Configuration and Credentials Reference Guide]
(https://docs.aws.amazon.com/credref/latest/refdocs/creds-config-files.html).
- PHP 7.1 or later
- Composer installed

## Additional resources

- [Amazon DynamoDB documentation](https://docs.aws.amazon.com/dynamodb)

---
Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
SPDX-License-Identifier: Apache-2.0
