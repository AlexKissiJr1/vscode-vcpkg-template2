#include <iostream>
#include <sqlite3.h>
#include <fmt/core.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/config/osconfig.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <optional>
#include <string>
#include <filesystem>

//#include <httplib.h>

using string_t = std::string;
using namespace std;

// Merly Urs Custom Parse Struct 
template<typename T>
struct Uri
{
public:
    T QueryString, Path, Protocol, Host, Port;

    static Uri Parse(T const& uri)
    {
        Uri result{};

        typedef typename T::const_iterator iterator_t;

        if (uri.length() == 0)
            return result;

        iterator_t uriEnd = uri.end();
        iterator_t queryStart = std::find(uri.begin(), uriEnd, static_cast<typename T::value_type>('?'));
        iterator_t protocolStart = uri.begin();
        iterator_t protocolEnd = std::find(protocolStart, uriEnd, static_cast<typename T::value_type>(':'));

        if (protocolEnd != uriEnd)
        {
            // looking for '://'
            T prot = &*(protocolEnd);
            if ((prot.length() > 3)
                && (prot[0] == static_cast<typename T::value_type>(':'))
                && (prot[1] == static_cast<typename T::value_type>('/'))
                && (prot[2] == static_cast<typename T::value_type>('/'))
                )
            {
                result.Protocol = T(protocolStart, protocolEnd);
                protocolEnd += 3;   //      ://
            }
            else
                protocolEnd = uri.begin();  // no protocol
        }
        else
            protocolEnd = uri.begin();  // no protocol

        // host
        iterator_t hostStart = protocolEnd;
        iterator_t pathStart = std::find(hostStart, uriEnd, static_cast<typename T::value_type>('/'));  // get pathStart

        iterator_t hostEnd = std::find(protocolEnd,
            (pathStart != uriEnd) ? pathStart : queryStart,
            static_cast<typename T::value_type>(':'));  // check for port

        result.Host = T(hostStart, hostEnd);

        if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == static_cast<typename T::value_type>(':')))  // we have a port
        {
            hostEnd++;
            iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
            result.Port = T(hostEnd, portEnd);
        }

        if (pathStart != uriEnd)
            result.Path = T(pathStart, queryStart);

        if (queryStart != uriEnd)
            result.QueryString = T(queryStart, uri.end());

        return result;

    }   // Parse
};  // 


bool getInfo(DcmFileFormat& fileformat, const DcmTagKey& tagKey, std::string& info) {
    OFString ofData;
    OFCondition status = fileformat.getDataset()->findAndGetOFString(tagKey, ofData);
    if (status.bad()) {
        std::cout << "get tag " << tagKey.toString() << DcmTag(tagKey).getTagName() << "error: " << status.text();
        return false;
    }
    info = ofData.c_str();
    return true;
}

void HttpRequest()
{
    httplib::Client cli("https://httpbin.org");

    {
        auto resp = cli.Get("/get");
        std::cout << "status: " << resp->status << std::endl;
        std::cout << resp->body << std::endl;
    }
    {
        auto resp = cli.Post("/post");
        std::cout << "status: " << resp->status << std::endl;
       // std::cout << resp->body << std::endl;

        auto js = json::parse(resp->body);
        std::cout << "User-Agent: " << js["headers"]["User-Agent"] << std::endl;
    }
}


int main() {
    /*
    // --- Test DCMTK (Static Linking)
    TCHAR NPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, NPath);
    std::cout << "Current work directory: " << NPath << std::endl;
    std::string dcmFile = (std::string(NPath) + "\\build\\assets\\0002.DCM");
    std::cout << dcmFile << std::endl;

    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(dcmFile.c_str());
    if (status.bad()) {
        std::cerr << std::string("Error: cannot read DICOM file: ") + status.text() << std::endl;
        std::cout << "TEsting alex" << std::endl;
        fmt::print("Tyler is amaizng");
        return 1;
    }

    std::string numOfFrames;
    if (!getInfo(fileformat, DCM_NumberOfFrames, numOfFrames)) return 1;
    std::cout << "numOfFrames of DCM file: " << numOfFrames << std::endl;

    // --- Test SQLite3 (Copy DLLs)
    sqlite3* db;
    char* zErrMsg = 0;
    int rc;

    rc = sqlite3_open("test.db", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
    sqlite3_close(db);
    */
    std::string url = "https://merlyserviceadmin.azurewebsites.net/api/VersionInfo?name=%2A&os=Windows";

    auto uri = Uri<string_t>::Parse(url);
    string_t address = uri.Host;
    
    if (address == "merlyserviceadmin.azurewebsites.net") address = "merly-mentor.ai:4300";
    httplib::SSLClient cli(address, 0);
    cli.set_follow_location(true);
    cli.set_follow_location(true);
    cli.set_connection_timeout(5, 0); // 5 sec
    cli.set_read_timeout(5, 0);       // 5 sec


    try {
        HttpRequest();
    }
    catch (exception& e) {
        std::cerr << e.what() <<std:: endl;
    }
    catch (...) {
        std::cerr << "catch ..." << std::endl;
    }
    
    return 0;
}
