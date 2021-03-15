#include "src/test.pb.h"

#include "pb_common.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

void setup() {

  Serial.begin(115200);

  uint8_t buffer[128];

  TestMessage message = TestMessage_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  message.test_number = 540;

  bool status = pb_encode(&stream, TestMessage_fields, &message);

  if (!status)
  {
      Serial.println("Failed to encode");
      return;
  }

  Serial.print("Message Length: ");
  Serial.println(stream.bytes_written);

  Serial.print("Message: ");

//  for(int i = 0; i<stream.bytes_written; i++){
//    Serial.print("%02X",buffer[i]);
//  }


}

void loop() {}
