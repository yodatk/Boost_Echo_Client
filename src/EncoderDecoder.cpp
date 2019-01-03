#include <utility>

//
// Created by tomer on 02/01/19.
//


#include <EncoderDecoder.h>
#include <boost/algorithm/string.hpp>
#include <locale>

using namespace std;


EncoderDecoder::EncoderDecoder() : commandDictionary(), zeroDelimiter('\0')  {

}

/**
 * Initialising the delimiter, and the values of the Messages's opcodes
 */
void EncoderDecoder::init() {
    //Register request = 1
    this->commandDictionary.insert(std::pair<string,short>("REGISTER",REGISTER));
    //Login request = 2
    this->commandDictionary.insert(std::pair<string,short>("LOGIN",LOGIN));
    //Logout request = 3
    this->commandDictionary.insert(std::pair<string,short>("LOGOUT",LOGOUT));
    //Follow request = 4
    this->commandDictionary.insert(std::pair<string,short>("FOLLOW",FOLLOW));
    //Post request = 5
    this->commandDictionary.insert(std::pair<string,short>("POST",POST));
    //PM request = 6
    this->commandDictionary.insert(std::pair<string,short>("PM",PM));
    //UserList request = 7
    this->commandDictionary.insert(std::pair<string,short>("USERLIST",USERLIST));
    //Stat request = 8
    this->commandDictionary.insert(std::pair<string,short>("STAT",STAT));
    this->zeroDelimiter = '\0';
}

    //region Encoding Functions

/**
* Converting a short number to array of chars
* @param num               short number to convert
* @param bytesArr          Char array to put the number into
*/
void EncoderDecoder::shortToBytes(short num, char bytesArr[]) {
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}


/**
* Convert the input string from the client to a char array to send to the server.
* @param input         String represent the user input
* @return              Char* represent a bytes array to send to the server to process
*/
std::vector<char> EncoderDecoder::stringToMessage(std::string input) {
    char ch_Opcode[2];
    std::vector<char> output;
    // taking the first word of the sentence to process which kind of request it is from the user
    std::string command = boost::to_upper_copy<std::string>(input.substr(0,input.find_first_of(" ")));
    input = input.substr(input.find_first_of(" ") + 1);
    //translating the first word to a Opcode using the commandDictionary.
    short opcode = this->commandDictionary.at(command);
    this->shortToBytes(opcode,ch_Opcode);
    switch (opcode) {
        case REGISTER:
            //register case
            registerAndLoginToMessage(input, ch_Opcode,output);
            break;
        case LOGIN:
            //login case
            registerAndLoginToMessage(input, ch_Opcode,output);
            break;
        case LOGOUT:
            //logout case
            output.push_back(ch_Opcode[0]);
            output.push_back(ch_Opcode[1]);
            break;
        case FOLLOW:
            //follow case
            followToMessage(input, ch_Opcode,output);
            break;
        case POST:
            //post case
            postOrStatToMessage(input, ch_Opcode,output);
            break;
        case PM:
            //pm case
            pmToMessage(input, ch_Opcode,output);
            break;
        case USERLIST:
            //User list case
            output.push_back(ch_Opcode[0]);
            output.push_back(ch_Opcode[1]);
            break;
        default:
            //stat case
            postOrStatToMessage(input, ch_Opcode,output);
            break;
    }
    return output;
}

/**
 * Part of the StringToMessage function
 * when the message is identified as a Login or Register request -->
 * processing the rest of the string by the user as a Login or Register (according to the OpCode) Message Type.
 * @param input                 String represent the input that was entered by the user.
 * @param ch_Opcode             char array represents the Opcode of this message.
 * @return      Char Array that represents the final Login or Register message
 */
void EncoderDecoder::registerAndLoginToMessage(std::string input, char *ch_Opcode, std::vector<char> &output) {
    //register case
    std::string userName(input.substr(0,input.find_first_of(" ")));
    input = input.substr(input.find_first_of(" ") + 1);
    std::string password(input.substr(0,input.find_first_of(" ")));
    //creating temp char* in the size of the opcode,username and password
    output.push_back(ch_Opcode[0]);
    output.push_back(ch_Opcode[1]);
    for (char i : userName) {
        //inserting all the chars of the user name
        output.push_back(i);
    }
    output.push_back(this->zeroDelimiter);
    for (char i : password) {
        //inserting all the chars from the password
        output.push_back(i);
    }
    output.push_back(this->zeroDelimiter);
}

/**
 * Part of the StringToMessage function
 * when the message is identified as a follow request --> processing the rest of the string by the user as a Follow Message Type.
 * @param input                 String represent the input that was entered by the user.
 * @param ch_Opcode             char array represents the Opcode of this message.
 * @return      Char Array that represents the final follow message
 */
void EncoderDecoder::followToMessage(std::string input, char *ch_Opcode, std::vector<char> &output) {
    char yesOrNo;
    //getting the "follow or not follow" from the string
    std::string followOrNot = input.substr(0,input.find_first_of(" "));
    input = input.substr(input.find_first_of(" ") + 1);
    if(followOrNot == "0"){
        //follow case
        yesOrNo = '0';
    }
    else{
        //unfollow case
        yesOrNo = '1';
    }
    //taking the number of users in the list from the user input
    short numberOfUsers = (short)std::stoi(input.substr(0,input.find_first_of(" ")));
    input = input.substr(input.find_first_of(" ") + 1);
    char* ch_numberOfUsers[2];
    this->shortToBytes(numberOfUsers,*ch_numberOfUsers);
    //creating a vector to hold the usernames to search in the server
    std::vector<string> names;
    int counter = 0;
    unsigned long end = input.find_first_of(" ");
    while (end != input.npos){
        //as long as there is still a user left to read --> adding it to the names vector
        std::string current = input.substr(0,input.find_first_of(" "));
        input = input.substr(input.find_first_of(" ") + 1);
        end = input.find_first_of(" ");
        counter +=current.length();
        names.push_back(current);
    }
    //the output needs to contains:
    //1.the opcode
    //2.the follow\unfollow
    //3.the names (names) and 0 between them (names.size)

    //inserting all the elements in the right order to the output array
    //inserting the opCode
    output.push_back(ch_Opcode[0]);
    output.push_back(ch_Opcode[1]);
    //inserting the yesOrNo char
    output.push_back(yesOrNo);
    for (auto &name : names) {
        //for each name in the vector
        for (char j : name) {
            //inserting all the letters of the user
            output.push_back(j);
        }
        //after each name --> putting the '\0' delimiter.
        output.push_back(this->zeroDelimiter);
    }
}

/**
 * Part of the FollowToMessage
 * combining all the char array that represents a Follow message,
 * and building one array that include all the information needed by the server.
 * @param ch_Opcode             Char array represent the first two bytes that are the Opcode of the message.
 * @param yesOrNo               Char that represent whether the the user want to follow or unfollow the people in the list.
 * @param names                 Vector of strings represent all the UserNames the current client want to follow or unfollow
 * @param output                Char array to bput all the given information in, in the correct order
 */
void EncoderDecoder::insertElementsToFollowInput(char *ch_Opcode, char &yesOrNo, std::vector<std::string> &names,std::vector<char> &output) {
    //inserting the opCode
    output.push_back(ch_Opcode[0]);
    output.push_back(ch_Opcode[1]);
    //inserting the yesOrNo char
    output.push_back(yesOrNo);
    for (auto &name : names) {
        //for each name in the vector
        for (char j : name) {
            //inserting all the letters of the user
            output.push_back(j);
        }
        //after each name --> putting the '\0' delimiter.
        output.push_back(this->zeroDelimiter);
    }
}

/**
 * Part of the StringToMessage function
 * when the message is identified as a Stat or Post request -->
 * processing the rest of the string by the user as a PM or Stat (according to the OpCode) Message Type.
 * @param input                 String represent the input that was entered by the user.
 * @param ch_Opcode             char array represents the Opcode of this message.
 * @return      Char Array that represents the final PM or Stat message
 */
void EncoderDecoder::postOrStatToMessage(std::string input, char *ch_Opcode, std::vector<char> &output) {

    //inserting the opcode to the array
    output.push_back(ch_Opcode[0]);
    output.push_back(ch_Opcode[1]);
    //inserting:
    //1. the useranme if it's a Stat message
    //2. the content if it's a post
    for (char i : input) {
        output.push_back(i);
    }
    //adding the '\0' delimiter in the end of the message
    output.push_back(this->zeroDelimiter);
}

/**
 * Part of the StringToMessage function
 * when the message is identified as a PM request --> processing the rest of the string by the user as a PM Message Type.
 * @param input                 String represent the input that was entered by the user.
 * @param ch_Opcode             char array represents the Opcode of this message.
 * @return      Char Array that represents the final PM message
 */
void EncoderDecoder::pmToMessage(std::string input, char *ch_Opcode, std::vector<char> &output) {
    //getting the user name to search in the server
    std::string userName(input.substr(0,input.find_first_of(" ")));
    //creating the output array
    input = input.substr(input.find_first_of(" ") + 1);
    //inserting the opcode to the output array
    output.push_back(ch_Opcode[0]);
    output.push_back(ch_Opcode[1]);
    //inserting the user name to the array
    for (char i : userName) {
        output.push_back(i);
    }
    //adding '\0' delimiter between the username to the content of the message
    output.push_back(this->zeroDelimiter);
    //adding all the content of the message to the output array
    for (char i : input) {
        output.push_back(i);
    }
    //adding the '\0' delimiter
    output.push_back(this->zeroDelimiter);
}

//endregion Encoding Functions

    //region Decoding Functions

/**
* Converting Char array to a Short number
* @param bytesArr              Char array to convert
* @return        Short number that is the bytes value of what was in the bytes array
*/
short EncoderDecoder::bytesToShort(char *bytesArr) {
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}


/**
* Convert the short number represents the opcode of the message to the message type string
* @param opcode            short number represent the opcode of the message type
* @return      string represents the message type
*/
std::string EncoderDecoder::messageToString(char *messageFromServer) {

    short incomingMessageOpcode = bytesToShort(messageFromServer);
    switch(incomingMessageOpcode){
        case NOTIFICATION:
            return "NOTIFICATION " + notificationToString(messageFromServer);
        case ACK:
            return "ACK " + ackToString(messageFromServer);
        default:
            return "ERROR " + errorToString(messageFromServer);

    }
}

/**
* part of the messageToString Function;
* convert the char array to String notification message to display on the screen to the client
* @param messageFromServer             Char Array that was received from the server
* @return              String representation of the message from the server.
*/
std::string EncoderDecoder::notificationToString(char *messageFromServer) {
    std::string output;
    char notificationType = messageFromServer[2];
    if(notificationType == '0'){
        //PM message
        output.append("PM ");
    }
    else{
        //Post Message
        output.append("Public ");
    }
    int index = 3;
    //inserting the posting username
    index = insertCharsToOutput(messageFromServer, output, index);
    output.append(" ");
    //inserting the content of the message
    index = insertCharsToOutput(messageFromServer, output, index);
    return output;
}

/**
* Insert to chars to string until it reaches the delimiter
* @param messageFromServer             Char* received by the server
* @param output                        String to add the chars to
* @param index                         current index of the "messageFromServer" array.
* @return
*/
int EncoderDecoder::insertCharsToOutput(char *messageFromServer, string &output, int index) {
    while(messageFromServer[index] != zeroDelimiter){
        output.append(to_string(messageFromServer[index]));
        index++;
    }
    return index;
}

/**
 * part of the messageToString Function;
 * convert the char array to String ack message to display on the screen to the client
 * @param messageFromServer             Char Array that was received from the server
 * @return              String representation of the message from the server.
 */
std::string EncoderDecoder::ackToString(char *messageFromServer) {
    short opcode = gettingShortFromCharArray(messageFromServer,2);
    switch(opcode){
        case FOLLOW:
            return std::to_string(opcode) + " " + followOrUserListAckToString(messageFromServer);
        case USERLIST:
            return std::to_string(opcode) + " " + followOrUserListAckToString(messageFromServer);
        case STAT:
            return std::to_string(opcode) + " " + statAckToString(messageFromServer);
        default:
            //in case it's one of the following:
            // 1.Register
            // 2.Login
            // 3.Logout
            // 4.Post
            // 5.Pm
            return std::to_string(opcode);

    }
}

/**
* Part of the "ackToString" functions.
* translating all the information in the given array from the server as it was an Follow Or UserList Ack Message.
* @param messageFromServer             Char array that was received from the server.
* @return      String representation of the given array as a Follow Or UserList Ack message.
*/
std::string EncoderDecoder::followOrUserListAckToString(char *messageFromServer) {
    std::string output;
    short numberOfUsers = gettingShortFromCharArray(messageFromServer,4);
    output.append(std::to_string(numberOfUsers));
    int index = 6;
    for(int i = 0 ; i<numberOfUsers;i++){
        output.append(" ");
        index = insertCharsToOutput(messageFromServer,output,index);
    }

    return output;
}

/**
 * Part of the "ackToString" functions.
 * translating all the information in the given array from the server as it was an stat Ack Message.
 * @param messageFromServer             Char array that was received from the server.
 * @return      String representation of the given array as a stat Ack message.
 */
std::string EncoderDecoder::statAckToString(char *messageFromServer) {
    std::string output;
    //adding number of posts.
    output.append(std::to_string(gettingShortFromCharArray(messageFromServer,4)));
    output.append(" ");
    //adding number of followers
    output.append(std::to_string(gettingShortFromCharArray(messageFromServer,6)));
    output.append(" ");
    //adding number of following
    output.append(std::to_string(gettingShortFromCharArray(messageFromServer,8)));
    return output;
}

/**
 * part of the messageToString Function;
 * convert the char array to String error message to display on the screen to the client
 * @param messageFromServer             Char Array that was recieved from the server
 * @return              String representation of the message from the server.
 */
std::string EncoderDecoder::errorToString(char *messageFromServer) {
    short opcode = gettingShortFromCharArray(messageFromServer, 2);
    //returning the number type of the error
    return std::to_string(opcode);
}

/**
 * Taking the first two chars from the given index from the given char array and convert them to short number.
 * @param input                 char array to take the number from
 * @param startIndex            integer index to start taking the number from
 * @return                      Short number that was taken from the first two bytes from the given array.
 */
short EncoderDecoder::gettingShortFromCharArray(char *input, int startIndex) {
    char* number[2];
    //getting the error type from the array.
    number[0] = &input[startIndex];
    startIndex++;
    number[1] = &input[startIndex];
    short output = bytesToShort(*number);
    return output;
}

//endregion Decoding Functions