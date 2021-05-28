#pragma once

enum class DICOM_eType {
  TYPE_AE, ///< Application Entity string 16 bytes maximum
  TYPE_AS, ///< Age String string 4 bytes fixed
  TYPE_AT, ///< Attribute Tag string 4 bytes fixed
  TYPE_CS, ///< Code String string 16 bytes maximum
  TYPE_DA, ///< Date string 8 bytes fixed
  TYPE_DS, ///< Decimal String string 16 bytes maximum
  TYPE_DT, ///< Date Time string 26 bytes maximum
  TYPE_FL, ///< Floating Point Single binary 4 bytes fixed
  TYPE_FD, ///< Floating Point Double binary 8 bytes fixed
  TYPE_IS, ///< Integer String string 12 bytes maximum
  TYPE_LO, ///< Long String string 64 chars maximum
  TYPE_LT, ///< Long Text string 1024 chars maximum
  TYPE_OB, ///< Other Byte
  TYPE_OW, ///< Other Word
  TYPE_OF, ///< Other Float
  TYPE_PN, ///< Person Name string 64 chars maximum
  TYPE_SH, ///< Short String string 16 chars maximum
  TYPE_SL, ///< Signed Long binary 4 bytes fixed
  TYPE_SQ, ///< Sequence of Items - -
  TYPE_SS, ///< Signed Short binary 2 bytes fixed
  TYPE_ST, ///< Short Text string 1024 chars maximum
  TYPE_TM, ///< Time string 16 bytes maximum
  TYPE_UI, ///< Unique Identifier (UID) string 64 bytes maximum
  TYPE_UL, ///< Unsigned Long binary 4 bytes fixed
  TYPE_US, ///< Unsigned Short binary 2 bytes fixed
  TYPE_UT, ///< Unlimited Text string 232-2
  TYPE_UN, ///< Unknown
  TYPE_Implicit ///< Implict File no type
};

static std::string DICOM_TypeStrings[28] = {
  "AE", // Application Entity string 16 bytes maximum
  "AS", // Age String string 4 bytes fixed
  "AT", // Attribute Tag string 4 bytes fixed
  "CS", // Code String string 16 bytes maximum
  "DA", // Date string 8 bytes fixed
  "DS", // Decimal String string 16 bytes maximum
  "DT", // Date Time string 26 bytes maximum
  "FL", // Floating Point Single binary 4 bytes fixed
  "FD", // Floating Point Double binary 8 bytes fixed
  "IS", // Integer String string 12 bytes maximum
  "LO", // Long String string 64 chars maximum
  "LT", // Long Text string 1024 chars maximum
  "OB", // Other Byte
  "OW", // Other Word
  "OF", // Other Float
  "PN", // Person Name string 64 chars maximum
  "SH", // Short String string 16 chars maximum
  "SL", // Signed Long binary 4 bytes fixed
  "SQ", // Sequence of Items - -
  "SS", // Signed Short binary 2 bytes fixed
  "ST", // Short Text string 1024 chars maximum
  "TM", // Time string 16 bytes maximum
  "UI", // Unique Identifier (UID) string 64 bytes maximum
  "UL", // Unsigned Long binary 4 bytes fixed
  "US", // Unsigned Short binary 2 bytes fixed
  "UT", // Unlimited Text string 232-2
  "UN", // Unknown
  "Implicit"
};
