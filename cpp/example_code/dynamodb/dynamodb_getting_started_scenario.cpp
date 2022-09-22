/*
   Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
   SPDX-License-Identifier: Apache-2.0
*/

#include "dyanamodb_samples.h"
#include <iostream>
#include <iomanip>
#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/CreateTableRequest.h>
#include <aws/dynamodb/model/DeleteTableRequest.h>
#include <aws/dynamodb/model/DescribeTableRequest.h>
#include <aws/dynamodb/model/GetItemRequest.h>
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/QueryRequest.h>
#include <aws/dynamodb/model/UpdateItemRequest.h>
#include <aws/dynamodb/model/BatchWriteItemRequest.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/http/HttpClient.h>
#include <fstream>
#include <zlib.h>
#include <array>

namespace AwsDoc {
    namespace DynamoDB {
        static const Aws::String MOVIE_TABLE_NAME("doc-example-table-movies");
        static const Aws::String YEAR_KEY("year");
        static const Aws::String TITLE_KEY("title");
        static const Aws::String INFO_KEY("info");
        static const Aws::String RATING_KEY("rating");
        static const Aws::String PLOT_KEY("plot");
        static const int PROVISIONED_THROUGHPUT_UNITS = 10;
        static const Aws::String ALLOCATION_TAG("dynamodb_scenario");

        static bool deleteDynamoTable(const Aws::String &tableName,
                                      const Aws::Client::ClientConfiguration &clientConfiguration);

        static Aws::String askQuestion(const Aws::String &string,
                                       std::function<bool(Aws::String)> test = [](
                                               const Aws::String &) -> bool { return true; });

        int askQuestionForInt(const Aws::String &string);

        float askQuestionForFloatRange(const Aws::String &string, float low, float high);

        static bool waitTableActive(const Aws::String &tableName,
                                    const Aws::Client::ClientConfiguration &clientConfiguration);

        static Aws::String getMovieJSON();

        Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>
        movieJsonViewToAttributeMap(const Aws::Utils::Json::JsonView &jsonView);

        void printMovieInfo(const Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>& movieMap);

        static int inflateZip(FILE *source, FILE *dest);

    } //  namespace DynamoDB
} // namespace AwsDoc

/*
 *
 * 1. Create a table with partition: year (N) and sort: title (S) (CreateTable)
 * 2. Add a new movie. (PutItem)
 * 3. Update the rating and plot of the movie by using an update expression. (UpdateItem with UpdateExpression + ExpressionAttributeValues args)
 * 4. Put movies in the table from moviedata.json--download it from the DynamoDB guide. OK to include manual download and unzip steps in your example README. This is a large file so limit the number of movies to 250 or so. (BatchWriteItem)
 * 5. Get a movie by Key (partition + sort) (GetItem)
 * 6. Use Query with a key condition expression to return all movies released in a given year. (Query + KeyConditionExpression arg)
 * 7. Use Scan to return movies released within a range of years. Show how to paginate data using ExclusiveStartKey. (Scan + FilterExpression)
 * 8. Delete a movie. (DeleteItem)
 * 9.Delete the table. (DeleteTable)
 */

bool AwsDoc::DynamoDB::dynamodbGettingStartedScenario(const Aws::Client::ClientConfiguration &clientConfiguration) {
    std::cout << std::setfill('*') << std::setw(88) << " " << std::endl;
    std::cout << "Welcome to the Amazon DynamoDB getting started demo." << std::endl;
    std::cout << std::setfill('*') << std::setw(88) << " " << std::endl;

    Aws::DynamoDB::DynamoDBClient dynamoClient(clientConfiguration);
#if 0
    bool movieTableAlreadyExisted = false;

    // 1. Create a table with partition: year (N) and sort: title (S) (CreateTable)
    {
        Aws::DynamoDB::Model::CreateTableRequest request;

        Aws::DynamoDB::Model::AttributeDefinition yearAttributeDefinition;
        yearAttributeDefinition.SetAttributeName(YEAR_KEY);
        yearAttributeDefinition.SetAttributeType(Aws::DynamoDB::Model::ScalarAttributeType::N);
        request.AddAttributeDefinitions(yearAttributeDefinition);

        Aws::DynamoDB::Model::AttributeDefinition titleAttributeDefinition;
        yearAttributeDefinition.SetAttributeName(TITLE_KEY);
        yearAttributeDefinition.SetAttributeType(Aws::DynamoDB::Model::ScalarAttributeType::S);
        request.AddAttributeDefinitions(yearAttributeDefinition);

        Aws::DynamoDB::Model::KeySchemaElement yearKeySchema;
        yearKeySchema.WithAttributeName(YEAR_KEY).WithKeyType(Aws::DynamoDB::Model::KeyType::HASH);
        request.AddKeySchema(yearKeySchema);

        Aws::DynamoDB::Model::KeySchemaElement titleKeySchema;
        yearKeySchema.WithAttributeName(TITLE_KEY).WithKeyType(Aws::DynamoDB::Model::KeyType::RANGE);
        request.AddKeySchema(yearKeySchema);

        Aws::DynamoDB::Model::ProvisionedThroughput throughput;
        throughput.WithReadCapacityUnits(PROVISIONED_THROUGHPUT_UNITS).WithWriteCapacityUnits(
                PROVISIONED_THROUGHPUT_UNITS);
        request.SetProvisionedThroughput(throughput);
        request.SetTableName(MOVIE_TABLE_NAME);

        std::cout << "Creating table '" << MOVIE_TABLE_NAME << "'..." << std::endl;
        const Aws::DynamoDB::Model::CreateTableOutcome &result = dynamoClient.CreateTable(request);
        if (!result.IsSuccess()) {
            if (result.GetError().GetErrorType() == Aws::DynamoDB::DynamoDBErrors::RESOURCE_IN_USE) {
                std::cout << "Table already exists." << std::endl;
                movieTableAlreadyExisted = true;
            }
            else {
                std::cerr << "Failed to create table: " << result.GetError().GetMessage();
                return false;
            }
        }
    }

    if (!movieTableAlreadyExisted) {
        std::cout << "Waiting for table '" << MOVIE_TABLE_NAME << "' to become active...." << std::endl;
        if (!AwsDoc::DynamoDB::waitTableActive(MOVIE_TABLE_NAME, clientConfiguration)) {
            deleteDynamoTable(MOVIE_TABLE_NAME, clientConfiguration);
            return false;
        }
        std::cout << "Table '" << MOVIE_TABLE_NAME << "' created and active." << std::endl;
    }

#if 0
    // 2. Add a new movie. (PutItem)
    Aws::String title;
    float rating;
    int year;
    Aws::String plot;
    {
        title = askQuestion("Enter the title of a movie you want to add to the table: ");
        year = askQuestionForInt("What year was it released? ");
        rating = askQuestionForFloatRange("On a scale of 1 - 10, how do you rate it? ", 1, 10);
        plot = askQuestion("Summarize the plot for me: ");

        Aws::DynamoDB::Model::PutItemRequest putItemRequest;
        putItemRequest.SetTableName(MOVIE_TABLE_NAME);

        putItemRequest.AddItem(YEAR_KEY, Aws::DynamoDB::Model::AttributeValue().SetN(year));
        putItemRequest.AddItem(TITLE_KEY, Aws::DynamoDB::Model::AttributeValue().SetS(title));

        // Create attribute for the info map.
        Aws::DynamoDB::Model::AttributeValue infoMapAttribute;

        std::shared_ptr<Aws::DynamoDB::Model::AttributeValue> ratingAttribute = Aws::MakeShared<Aws::DynamoDB::Model::AttributeValue>(ALLOCATION_TAG.c_str());
        ratingAttribute->SetN(rating);
        infoMapAttribute.AddMEntry(RATING_KEY, ratingAttribute);

        std::shared_ptr<Aws::DynamoDB::Model::AttributeValue> plotAttibute = Aws::MakeShared<Aws::DynamoDB::Model::AttributeValue>(ALLOCATION_TAG.c_str());
        plotAttibute->SetS(plot);
        infoMapAttribute.AddMEntry(PLOT_KEY, plotAttibute);

        putItemRequest.AddItem(INFO_KEY, infoMapAttribute);

        Aws::DynamoDB::Model::PutItemOutcome outcome = dynamoClient.PutItem(putItemRequest);
        if (!outcome.IsSuccess()) {
            std::cerr << "Failed to add an item: " << outcome.GetError().GetMessage();
            deleteDynamoTable(MOVIE_TABLE_NAME, clientConfiguration);
            return false;
        }
    }

    std::cout << "\nAdded '" << title << "' to '" << MOVIE_TABLE_NAME << "'." << std::endl;

    // 3. Update the rating and plot of the movie by using an update expression.
    // (UpdateItem with UpdateExpression + ExpressionAttributeValues args)
    {
        rating = askQuestionForFloatRange(Aws::String("\nLet's update your movie.\nYou rated it  ") + std::to_string(rating)
                + ", what new would you give it? ", 1, 10);
        plot = askQuestion(Aws::String("You summarized the plot as '") + plot + "'.\nWhat would you say now? ");

        Aws::DynamoDB::Model::UpdateItemRequest request;
        request.SetTableName(MOVIE_TABLE_NAME);
        request.AddKey(TITLE_KEY, Aws::DynamoDB::Model::AttributeValue().SetS(title));
        request.AddKey(YEAR_KEY, Aws::DynamoDB::Model::AttributeValue().SetN(year));
        std::stringstream  expressionStream;
        expressionStream << "set " << INFO_KEY << "." << RATING_KEY << " =:r, "
            << INFO_KEY << "." << PLOT_KEY << " =:p";
        request.SetUpdateExpression(expressionStream.str());
        request.SetExpressionAttributeValues({
                                                     {":r", Aws::DynamoDB::Model::AttributeValue().SetN(rating)},
                                                     {":p", Aws::DynamoDB::Model::AttributeValue().SetS(plot)}
                                             });

        request.SetReturnValues(Aws::DynamoDB::Model::ReturnValue::UPDATED_NEW);

        const Aws::DynamoDB::Model::UpdateItemOutcome& result = dynamoClient.UpdateItem(request);
        if (!result.IsSuccess())
        {
            std::cerr << "Error updating movie " + result.GetError().GetMessage() << std::endl;
            deleteDynamoTable(MOVIE_TABLE_NAME, clientConfiguration);
            return false;
        }
    }

    std::cout << "\nUpdated '" << title << "' with new attributes:" << std::endl;

#endif
    if (!movieTableAlreadyExisted) {
       // 4. Put 250 movies in the table from moviedata.json (BatchWriteItem)
        const size_t MAX_SIZE_FOR_BATCH_WRITE = 25;
        const size_t MOVIES_TO_WRITE = 10 * MAX_SIZE_FOR_BATCH_WRITE;
        Aws::String jsonString = getMovieJSON();
        if (!jsonString.empty()) {
            Aws::Utils::Json::JsonValue json(jsonString);
            Aws::Utils::Array<Aws::Utils::Json::JsonView> movieJsons = json.View().AsArray();
            Aws::Vector<Aws::DynamoDB::Model::WriteRequest> writeRequests;

            for (size_t i = 0; i < MOVIES_TO_WRITE && i < movieJsons.GetLength(); ++i) {
                writeRequests.push_back(Aws::DynamoDB::Model::WriteRequest());
                Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue> putItems = movieJsonViewToAttributeMap(
                        movieJsons[i]);
                Aws::DynamoDB::Model::PutRequest putRequest;
                putRequest.SetItem(putItems);
                writeRequests.back().SetPutRequest(putRequest);
                if (writeRequests.size() == MAX_SIZE_FOR_BATCH_WRITE)
                {
                    Aws::DynamoDB::Model::BatchWriteItemRequest request;
                    request.AddRequestItems(MOVIE_TABLE_NAME, writeRequests);
                    const Aws::DynamoDB::Model::BatchWriteItemOutcome &outcome = dynamoClient.BatchWriteItem(request);
                    if (!outcome.IsSuccess()) {
                        std::cerr << "Unable to batch write movie data: " << outcome.GetError().GetMessage()
                                  << std::endl;
                        writeRequests.clear();
                        break;
                    }
                    else {
                        std::cout << "Added batch of " << writeRequests.size() << " movies to the database."
                                  << std::endl;
                    }
                    writeRequests.clear();
                }
            }
        }
    }
#endif
    std::cout << std::setfill('*') << std::setw(88) << " " << std::endl;

    // 5. Get a movie by Key (partition + sort) (GetItem)
    {
        Aws::String titleToGet("World War Z");
   //     Aws::String answer = askQuestion(Aws::String("Let's move on...do you want to get info about '" + titleToGet + "'? (y/n) "));
        if (false) //(answer == "y")
        {
            Aws::DynamoDB::Model::GetItemRequest request;
            request.SetTableName(MOVIE_TABLE_NAME);
            request.AddKey(TITLE_KEY, Aws::DynamoDB::Model::AttributeValue().SetS(titleToGet));
            request.AddKey(YEAR_KEY, Aws::DynamoDB::Model::AttributeValue().SetN(2013));

             const Aws::DynamoDB::Model::GetItemOutcome& result = dynamoClient.GetItem(request);
            if (!result.IsSuccess())
            {
                std::cout << "Error " << result.GetError().GetMessage();
                deleteDynamoTable(MOVIE_TABLE_NAME, clientConfiguration);
                return false;
            }
            else
            {
                const Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>& item = result.GetResult().GetItem();
                if (item.size() > 0)
                {
                    std::cout << "\nHere's what I found:" << std::endl;
                    printMovieInfo(item);
                }
                else
                {
                    std::cout << "The movie was not found in the database." << std::endl;
                }
            }
        }
    }

    // Use Query with a key condition expression to return all movies released in a given year. (Query + KeyConditionExpression arg)
    {
        Aws::DynamoDB::Model::QueryRequest req;

        req.SetTableName(MOVIE_TABLE_NAME);

        // "year" is a reserved keyword and must be replaced with an
        // expression attribute name
        req.SetKeyConditionExpression( "#dynobase_year = :valueToMatch");
        req.SetExpressionAttributeNames({{"#dynobase_year", YEAR_KEY}});

        // Set Expression AttributeValues
        Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue> attributeValues;
        attributeValues.emplace(":valueToMatch", Aws::DynamoDB::Model::AttributeValue().SetN(2013));

        req.SetExpressionAttributeValues(attributeValues);

        // Perform Query operation
        const Aws::DynamoDB::Model::QueryOutcome& result = dynamoClient.Query(req);
        if (result.IsSuccess())
        {
            // Reference the retrieved items
            const Aws::Vector<Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>>& items = result.GetResult().GetItems();
            if(items.size() > 0)
            {
                std::cout << "Number of items retrieved from Query: " << items.size() << std::endl;
                //Iterate each item and print
                for(const auto &item: items)
                {
                    std::cout << "******************************************************" << std::endl;
                    // Output each retrieved field and its value
                    for (const auto& i : item)
                        std::cout << i.first << ": " << i.second.GetS() << std::endl;
                }
            }
            else
            {
                std::cout << "No item found in table: " << MOVIE_TABLE_NAME << std::endl;
            }
        }
        else
        {
            std::cout << "Failed to Query items: " << result.GetError().GetMessage();
        }
    }

    return deleteDynamoTable(MOVIE_TABLE_NAME, clientConfiguration);
}

bool AwsDoc::DynamoDB::deleteDynamoTable(const Aws::String &tableName,
                                         const Aws::Client::ClientConfiguration &clientConfiguration) {
#if 0
    Aws::DynamoDB::DynamoDBClient dynamoClient(clientConfiguration);

    Aws::DynamoDB::Model::DeleteTableRequest request;
    request.SetTableName(tableName);

    const Aws::DynamoDB::Model::DeleteTableOutcome &result = dynamoClient.DeleteTable(request);
    if (result.IsSuccess()) {
        std::cout << "Your Table \"" << result.GetResult().GetTableDescription().GetTableName() << " was deleted!\n";
    }
    else {
        std::cerr << "Failed to delete table: " << result.GetError().GetMessage();
    }

    return result.IsSuccess();
#else
    return true;
#endif
}


bool AwsDoc::DynamoDB::waitTableActive(const Aws::String &tableName,
                                       const Aws::Client::ClientConfiguration &clientConfiguration) {
    Aws::DynamoDB::DynamoDBClient client(clientConfiguration);
    // Repeatedly call DescribeTable until table is ACTIVE
    Aws::DynamoDB::Model::DescribeTableRequest request;
    request.SetTableName(tableName);

    int count = 0;
    while (count < 20) {
        const Aws::DynamoDB::Model::DescribeTableOutcome &result = client.DescribeTable(request);
        if (!result.IsSuccess()) {
            std::cerr << "Error DynamoDB::waitTableActiveconst "
                      << result.GetError().GetMessage() << std::endl;
            return false;
        }
        else {
            Aws::DynamoDB::Model::TableStatus status = result.GetResult().GetTable().GetTableStatus();

            if (Aws::DynamoDB::Model::TableStatus::ACTIVE == status) {
                return true;
            }
            else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        count++;
    }
    return false;
}

Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>
AwsDoc::DynamoDB::movieJsonViewToAttributeMap(const Aws::Utils::Json::JsonView &jsonView) {
    Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue> result;

    if (jsonView.KeyExists(YEAR_KEY)) {
        result[YEAR_KEY].SetN(jsonView.GetInteger(YEAR_KEY));
    }
    if (jsonView.KeyExists(TITLE_KEY)) {
        result[TITLE_KEY].SetS(jsonView.GetString(TITLE_KEY));
    }
    if (jsonView.KeyExists(INFO_KEY)) {
        Aws::Map<Aws::String, const std::shared_ptr<Aws::DynamoDB::Model::AttributeValue>> infoMap;
        Aws::Utils::Json::JsonView infoView = jsonView.GetObject(INFO_KEY);
        if (infoView.KeyExists(RATING_KEY)) {
            std::shared_ptr<Aws::DynamoDB::Model::AttributeValue>  attributeValue = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
            attributeValue->SetN(infoView.GetDouble(RATING_KEY));
            infoMap.emplace(std::make_pair(RATING_KEY, attributeValue));
         }
        if (infoView.KeyExists(PLOT_KEY)) {
            std::shared_ptr<Aws::DynamoDB::Model::AttributeValue> attributeValue = std::make_shared<Aws::DynamoDB::Model::AttributeValue>();
            attributeValue->SetS(infoView.GetString(PLOT_KEY));
            infoMap.emplace(std::make_pair(PLOT_KEY, attributeValue));
        }

        result[INFO_KEY].SetM(infoMap);
    }

    return result;
}


Aws::String AwsDoc::DynamoDB::askQuestion(const Aws::String &string,
                                          std::function<bool(Aws::String)> test) {
    Aws::String result;
    do {
        std::cout << string;
        std::getline(std::cin, result);
        if (result.empty()) {
            std::cout << "Please enter some text." << std::endl;
        }
        if (!test(result)) {
            continue;
        }
    } while (result.empty());

    return result;
}

int AwsDoc::DynamoDB::askQuestionForInt(const Aws::String &string) {
    Aws::String resultString = askQuestion(string, [](const Aws::String &string1) -> bool {
        try {
            std::atoi(string1.c_str());
            return true;
        }
        catch (std::invalid_argument) {
            return false;
        }
    });

    int result = 0;
    try {
        result = std::atoi(resultString.c_str());
    }
    catch (std::invalid_argument) {
        std::cerr << "DynamoDB::askQuestionForInt string not an int "
                  << resultString << std::endl;
    }
    return result;
}

float AwsDoc::DynamoDB::askQuestionForFloatRange(const Aws::String &string, float low, float high) {
    Aws::String resultString = askQuestion(string, [low, high](const Aws::String &string1) -> bool {
        try {
            float number = std::atof(string1.c_str());
            return number >= low && number <= high;
        }
        catch (std::invalid_argument) {
            return false;
        }
    });
    float result = 0;
    try {
        result = std::atof(resultString.c_str());
    }
    catch (std::invalid_argument) {
        std::cerr << "DynamoDB::askQuestionForFloatRange string not an int "
                  << resultString << std::endl;
    }

    return result;
}

void AwsDoc::DynamoDB::printMovieInfo(const Aws::Map<Aws::String, Aws::DynamoDB::Model::AttributeValue>& movieMap)
{
    {
        auto const &iter = movieMap.find(TITLE_KEY);
        if (iter != movieMap.end()) {
            std::cout << "Movie title: '" + iter->second.GetS() << "'." << std::endl;
        }
    }

    {
        auto const &iter = movieMap.find(YEAR_KEY);
        if (iter != movieMap.end()) {
            std::cout << "    Year: " + iter->second.GetN() << "." << std::endl;
        }
    }

    {
        auto const &iter = movieMap.find(INFO_KEY);
        if (iter != movieMap.end()) {
            Aws::Map<Aws::String, const std::shared_ptr<Aws::DynamoDB::Model::AttributeValue>> infoMap =
                    iter->second.GetM();

            auto const &ratingIter = infoMap.find(RATING_KEY);
            if (ratingIter != infoMap.end())
            {
                std::cout << "    Rating: " + ratingIter->second->GetN() << "." << std::endl;
            }

            auto const &plotIter = infoMap.find(PLOT_KEY);
            if (plotIter != infoMap.end())
            {
                std::cout << "    Synopsis: " + plotIter->second->GetS() << "." << std::endl;
            }
        }
    }
}




#ifndef TESTING_BUILD

int main(int argc, char **argv) {
    Aws::SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
    InitAPI(options);

    {
        Aws::Client::ClientConfiguration clientConfig;
        AwsDoc::DynamoDB::dynamodbGettingStartedScenario(clientConfig);
    }
    return 0;
}

#endif // TESTING_BUILD

static void zerr(int ret);

Aws::String AwsDoc::DynamoDB::getMovieJSON() {
    const Aws::String JSON_FILE_NAME("moviedata.json");
    Aws::String result;
#if 0
    Aws::Client::ClientConfiguration config;

    auto httpClient = Aws::Http::CreateHttpClient(config);
    auto request = Aws::Http::CreateHttpRequest(Aws::String("https://docs.aws.amazon.com/amazondynamodb/latest/developerguide/samples/moviedata.zip"),
                                                Aws::Http::HttpMethod::HTTP_GET, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);
    request->SetUserAgent("curl/7.79.1");
    auto response = httpClient->MakeRequest(request);
    Aws::String result;

    const Aws::String ZIP_FILE_NAME("movie.zip");
     if (Aws::Http::HttpResponseCode::OK == response->GetResponseCode())
    {
        {
            std::ofstream outStream(ZIP_FILE_NAME);
            outStream << response->GetResponseBody().rdbuf();
        }
        FILE* src = fopen(ZIP_FILE_NAME.c_str(), "r");
        FILE* dst = fopen(JSON_FILE_NAME.c_str(), "w");

        int zipResult = inflateZip(src, dst);
        if(zipResult == Z_OK)
        {
            std::cout << "Successful deflate" << std::endl;
        }
        else
        {
            zerr(zipResult);
        }
    }
    else{
        std::cout << response->GetClientErrorMessage() << std::endl;
    }
#endif
    std::ifstream movieData(JSON_FILE_NAME);
    std::array<char, 1024> buffer;
    while (movieData) {
        movieData.read(&buffer[0], buffer.size() - 2);
        buffer[movieData.gcount()] = 0;
        result += &buffer[0];
    }

    return result;
}


/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int AwsDoc::DynamoDB::inflateZip(FILE *source, FILE *dest) {
    const int CHUNK = 16384;
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void) inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void) inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void) inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void) inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret) {
    fputs("zpipe: ", stderr);
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
        case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
        case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
        case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
    }
}

