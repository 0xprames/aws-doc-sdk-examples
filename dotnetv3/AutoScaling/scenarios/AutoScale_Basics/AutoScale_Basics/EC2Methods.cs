﻿// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier:  Apache-2.0

namespace AutoScale_Basics
{
    // snippet-start:[AutoScale.dotnetv3.AutoScale_Basics.EC2Methods]
    using Amazon.EC2;
    using Amazon.EC2.Model;

    /// <summary>
    /// The methods in this class create and delete an Amazon Elastic Compute
    /// Cloud (Amazon EC2) launch template for use by the AWS Auto Scaling
    /// scenario.
    /// </summary>
    public class EC2Methods
    {
        // Create a new Amazon Elastic Compute Cloud (Amazon EC2) template.
        public static async Task<string> CreateLaunchTemplateAsync(
            string imageId,
            string instanceType,
            string launchTemplateName)
        {
            var client = new AmazonEC2Client();

            var request = new CreateLaunchTemplateRequest
            {
                LaunchTemplateData = new RequestLaunchTemplateData
                {
                    ImageId = imageId,
                    InstanceType = instanceType,
                },
                LaunchTemplateName = launchTemplateName,
            };

            var response = await client.CreateLaunchTemplateAsync(request);

            return response.LaunchTemplate.LaunchTemplateId;
        }

        // Delete an Amazon Elastic Compute Cloud (Amazon EC2) launch template.
        public static async Task<string> DeleteLaunchTemplateAsync(string launchTemplateId)
        {
            var client = new AmazonEC2Client();

            var request = new DeleteLaunchTemplateRequest
            {
                LaunchTemplateId = launchTemplateId,
            };

            var response = await client.DeleteLaunchTemplateAsync(request);
            return response.LaunchTemplate.LaunchTemplateName;
        }

        // Get the details of an Amazon Elastic Cloud (Amazon EC2) launch template.
        public static async Task<bool> DescribeLaunchTemplateAsync(string launchTemplateName)
        {
            var client = new AmazonEC2Client();

            var request = new DescribeLaunchTemplatesRequest
            {
                LaunchTemplateNames = new List<string> { launchTemplateName, },
            };

            var response = await client.DescribeLaunchTemplatesAsync(request);

            if (response.LaunchTemplates != null)
            {
                response.LaunchTemplates.ForEach(template =>
                {
                    Console.Write($"{template.LaunchTemplateName}\t");
                    Console.WriteLine(template.LaunchTemplateId);
                });

                return true;
            }

            return false;
        }
    }

    // snippet-end:[AutoScale.dotnetv3.AutoScale_Basics.EC2Methods]
}
