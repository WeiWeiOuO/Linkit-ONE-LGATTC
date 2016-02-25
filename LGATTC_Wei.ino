#include <rgb_lcd.h>
#include <Wire.h>

#include <LGATT.h>
#include <LGATTClient.h>
#include <LGATTServer.h>

int i = 0;
int j = 0;

// create a uuid for app, this uuid id is to identify a client
static LGATTUUID test_uuid("B4B4B4B4-B4B4-B4B4-B4B4-B4B4B4B4B4B4");

// specified / prefered service uuid
// 128bit format for your cared service uuid
LGATTUUID uuidService("E20A39F4-73F5-4BC4-A12F-17D1AD07A961");

// 16bit format for your cared service uuid
// LGATTUUID uuidService = 0x180F; // battery service

LGATTClient c;
rgb_lcd lcd;

void setup() {
    Serial.begin(9600);
    //while(!Serial.available());
    lcd.begin(16,2);
    Serial.println("[LGATTC ino]setup.");
    lcd.clear();
    lcd.print("Setup");
    delay(1000);
    lcd.clear();
    lcd.print("begin");
    
    // GATT central begin to start
    if (c.begin(test_uuid))
    {
        //Serial.println("[LGATTC ino]begin() success to register.");
    }
    else
    {
        Serial.println("[LGATTC ino]begin() failed to register.");
        lcd.clear();
        lcd.print("Fail begin()");
        delay(0xffffffff);
    }
}

boolean descInProcessing(const LGATTUUID &serviceUUID,
            boolean isPrimary,
            const LGATTUUID& characteristicUUID
            )
{
    boolean ret1;
    LGATTUUID descriptorUUID;

    // get descriptor for current characteristic
    ret1 = c.getDescriptorInfo(serviceUUID, isPrimary, characteristicUUID, descriptorUUID);
    if (ret1)
    {
        Serial.print("[LGATTC ino]descriptor uuid :");
        Serial.print(descriptorUUID);
        Serial.println();
    }
    else
        return false;
    return true;
}

void charInProcessing(const LGATTUUID &serviceUUID, boolean isPrimary, const LGATTUUID& characteristicUUID, LGATT_CHAR_PROPERTIES properties)
{
    LGATTAttributeValue attrValue = {0};

    //Serial.printf("[LGATTC ino] Now is doing the Properties: 0x%x", properties);
    //Serial.println();
    //Serial.printf("0x%x,0x%x,0x%x,0x%x",LGATT_CHAR_PROP_NOTIFY,LGATT_CHAR_PROP_INDICATE,LGATT_CHAR_PROP_READ,LGATT_CHAR_PROP_WRITE);
    //Serial.println();

    //notification or indication
    //if ((properties & LGATT_CHAR_PROP_NOTIFY) == LGATT_CHAR_PROP_NOTIFY||(properties & LGATT_CHAR_PROP_INDICATE) == LGATT_CHAR_PROP_INDICATE)
    if (properties == LGATT_CHAR_PROP_NOTIFY || properties == LGATT_CHAR_PROP_INDICATE)
    {
        Serial.println("[LGATTC ino] notification or indication");

        // enable notification first
        if (!c.setupNotification(true, serviceUUID, isPrimary, characteristicUUID))
        {
            Serial.println("[LGATTC ino]notification already enabled");
        }

        // query if the notification has come.
        LGATTAttributeValue value = {0};//This is the value store the Notification value
        Serial.println("[LGATTC ino]query notification data");
        if (c.queryNotification(serviceUUID, isPrimary, characteristicUUID, value))
        {
            Serial.printf("[LGATTC ino]notification data[%s][%d]", value.value, value.len);
            Serial.println();
        }else{
          Serial.println("[LGATTC ino] Client can't not find the notification data");
        }

    }

    // read characteristic
    //if ((properties & LGATT_CHAR_PROP_READ) == LGATT_CHAR_PROP_READ)
    if (properties == LGATT_CHAR_PROP_READ)
    {
        Serial.println("[LGATTC ino] read characteristic");
        memset(&attrValue, 0, sizeof(attrValue));
        if (c.readCharacteristic(serviceUUID, isPrimary, characteristicUUID, attrValue))
        {
            Serial.print("[LGATTC ino]read char :");
            Serial.print((char*)attrValue.value);
            Serial.println();
        }
        else
        {
            Serial.print("[LGATTC ino]read char [FAILED]");
            Serial.println();
        }
    }

    // write characteristic
    //if ((properties & LGATT_CHAR_PROP_WRITE) == LGATT_CHAR_PROP_WRITE)
    if (properties == LGATT_CHAR_PROP_WRITE)
    {
        /* you can prepare data for the specified characteristic with write property here */
        i++; //test msg
        Serial.println("[LGATTC ino] write characteristic");
        char szbuf[] = "I'm tku YOU WEI LIN MSG"; // somedata will be sent to prepherial
        //sprintf(szbuf,"MSG:%s",i);
        
        Serial.printf("[LGATTC ino] Client will write [%s] to Server.",szbuf);
        Serial.println();
        memset(&attrValue, 0, sizeof(attrValue));
        memcpy(attrValue.value, szbuf, strlen(szbuf));
        attrValue.len = strlen(szbuf);
        if (c.writeCharacteristic(serviceUUID, isPrimary, characteristicUUID, attrValue))
        {
            Serial.print("[LGATTC ino]written data :");
            Serial.print((char*)attrValue.value);
            Serial.print(", len:");
            Serial.print(attrValue.len);
            Serial.println();
        }
        else
        {
            Serial.print("[LGATTC ino]write data [FAILED]");
            Serial.println();
        }
    }

}


void serviceInProcessing(const LGATTUUID &inputServiceUUID)
{
    // query all services, to find if your specified service exist or not.
    int num = c.getServiceCount();
    // enum all of the services
    Serial.print("[LGATTC ino] Client want to find this LGATTService uuid :");
    Serial.print(inputServiceUUID);
    Serial.println();

    for (i = 0; i < num; i++)
    {
        LGATTUUID serviceUUID;
        boolean isPrimary = false;
        /*Serial.print("[LGATTC ino]serviceInProcessing service uuid :");
        Serial.print(inputServiceUUID);
        Serial.println();*/

        // service uuid matched
        if (c.getServiceInfo(i, serviceUUID, isPrimary) &&
            inputServiceUUID == serviceUUID)
        {
            //Serial.print("[LGATTC ino] Client found service uuid prefered on LGATTServer :");
            //Serial.print(serviceUUID);
            //Serial.println();
            boolean ret;

            while (1)
            {
                delay(50);
                LGATTUUID characteristicUUID;
                LGATT_CHAR_PROPERTIES properties = 0;

                // polling all characteristics of current service
                ret = c.getCharacteristicInfo(serviceUUID, isPrimary, characteristicUUID, properties);
                if (ret)
                {
                    // characteristic processing here
                    charInProcessing(serviceUUID, isPrimary, characteristicUUID, properties);
                    while (1)
                    {
                        // polling all descriptors for current characteristic
                        if (!descInProcessing(serviceUUID, isPrimary, characteristicUUID)){
                            break;
                        }
                    }
                }
                else
                    break;


            };
            break;
        }else{
          //Serial.print("[LGATTC ino] Client find the service UUID on LGATTServer:");
          //Serial.print(serviceUUID);
          //Serial.println();
          delay(50);
        }

    }

}

boolean hasFoundSpencifiedBleDev(const LGATTDeviceInfo &info)
{
    // you can check your known remote device address here, or else the first descovered ble device will be connected.
    //e4:1f:7e:f4:e2:70 and e0:cd:50:b0:34:2c are my Server
    int def_addr[6] = {0x70,0xe2,0xf4,0x7e,0x1f,0xe4};
    int def_addr2[6] = {0x2c,0x34,0xb0,0x50,0xcd,0xe0};
    bool is_def_device = false;

    //cheak if the addr is my know remote device
    for(int c_dev=0;c_dev<6;c_dev++){
      if(info.bd_addr.addr[c_dev] == def_addr[c_dev]){
        is_def_device = true;
      }else if(info.bd_addr.addr[c_dev] == def_addr2[c_dev]){
        is_def_device = true;
      }else{
        is_def_device = false;
        break;
      }      
    }

    if(is_def_device)
    {
       return true;
    }
    return false;
}
void loop()
{
    // scan device
    //Serial.println("[LGATTC ino]loop::start to scan.");
    int num = 0;
    //This represents the information of a remote Bluetooth 4.0 peripheral device.
    LGATTDeviceInfo info = {0};


    while (1)
    {
        // scan ble devices
        num = c.scan(6); //The number of Bluetooth 4.0 devices that have been capture by scan().
        int found = 0;
        Serial.printf("[LGATTC ino]scan num [%d]", num);
        lcd.clear();
        lcd.write("Find %d dev.",num);
        delay(700);
        if(num==0){
          Serial.print(", Can not find any Bluetooth 4.0 device.");
        }
          
        Serial.println();

        // polling all found devices

        for (i = 0; i < num; i++)
        {
            c.getScanResult(i, info);
            /*Serial.printf("[LGATTC ino][num = %d]dev address : [%x:%x:%x:%x:%x:%x] rssi [%d]",
            i,info.bd_addr.addr[5], info.bd_addr.addr[4], info.bd_addr.addr[3], info.bd_addr.addr[2], info.bd_addr.addr[1], info.bd_addr.addr[0],
            info.rssi);
            Serial.println();*/

            lcd.write("Scan device.");

            // check if the current ble device is the one you prefered.
            if (hasFoundSpencifiedBleDev(info))
            {
                found = 1;
                break;
            }

        }
        if (found)
        {
            Serial.println("[LGATTC ino]Find the prefered device.");
            lcd.clear();
            lcd.print("Find the deivce!");
            break;
        }
        delay(500);
        Serial.println("[LGATTC ino]client can't find the prefered server!!");
        lcd.clear();
        lcd.print("Not Find device.");
    }

    //Serial.println("[LGATTC ino]loop::start to connect.");

    if (!c.connect(info.bd_addr)) // search all services till timeout or searching done.
    {
        Serial.println("[LGATTC ino]begin() failed to connect.");
        lcd.clear();
        lcd.print("begin() fail.");
        //delay(0xffffffff);
    }
    else{
      Serial.println("[LGATTC ino]begin() success to connect.");  
    }
    

    // central working here, serviceInProcessing is a function query all characteristics.
    // it make a chance for you to do some actions to the one or more characteristics.
    // such as read / write them.
    int times = 3;
    while (times--)
    {
        serviceInProcessing(uuidService);
        delay(1000);
    }

    // disconnect the remote
    c.disconnect(info.bd_addr);
    Serial.println("[LGATTC ino] Disconnected");

    // finished all
    if (!c.end())
    {
        Serial.println("[LGATTC ino] failed to end");
    }
    Serial.println("[LGATTC ino] ended");

    //delay(1000);

    int cd = 10;
    while(cd--){
      Serial.printf("Count Down: %d",cd);
      Serial.println();
      lcd.clear();
      lcd.write("CD:%d",cd);
      delay(1000);      
    }

    Serial.println("[LGATTC ino] ReStart!");
    
    // re-start again
    if (!c.begin(test_uuid))
    {
        Serial.println("[LGATTC ino] failed to begin");
    }
    //Serial.println("[LGATTC ino] Begined");
    //delay(0xffffffff);


    
}
