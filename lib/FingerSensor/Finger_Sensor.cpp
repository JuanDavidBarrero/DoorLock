#include "Finger_Sensor.h"

FingerSensor::FingerSensor(/* args */)
{
}

void FingerSensor::initFingerSentor()
{
    finger.begin(57600);
    if (!finger.verifyPassword())
    {
        Serial.print("Did NOT find finger print sensor \n");
        while (true)
        {
            Serial.print("Please restart the ESP32\n");
            delay(1000);
        }
    }
    Serial.print("Found finger print sensor \n");
}

int FingerSensor::enrollFinger(int id)
{
    if (id == 0)
        return -1;
    Serial.printf("Enrolling ID # %i \n", id);
    int p = -1;

    Serial.printf("Waiting for valid finger to enroll as # %i \n", id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }
}

int FingerSensor::verifyFinger(int id)
{
    Serial.println("Remove finger");
    delay(2000);

    int p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.printf("ID %i \n", id);
    p = -1;
    Serial.println("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.printf("Creating model for # %i \n", id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println("Fingerprints did not match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    Serial.printf("ID %id \n", id);

    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    return true;
}

bool FingerSensor::checkDB()
{
    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        Serial.print("Sensor doesn't contain any fingerpirnt data. please run the 'enroll' exmaple\n");
        return false;
    }

    return true;
}

int FingerSensor::identifingFinger()
{
    int p = finger.getImage();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)
        return -1;

    return finger.fingerID;
}

int FingerSensor::deleteFingerPrintFromDB(int id)
{
    uint8_t p = -1;

    p = finger.deleteModel(id);

    if (p == FINGERPRINT_OK)
    {
        Serial.println("Deleted!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not delete in that location");
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
    }
    else
    {
        Serial.print("Unknown error: 0x");
        Serial.println(p, HEX);
    }

    return p;
}
