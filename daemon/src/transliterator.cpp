#include "transliterator.h"
#include <unicode/unistr.h>
#include <unicode/translit.h>
#include <QDebug>

QString Transliterator::convert(const QString& input) {
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString uInput = icu::UnicodeString::fromUTF8(icu::StringPiece(input.toUtf8().constData()));

    if (U_FAILURE(status)) {
        qWarning() << "Transliterator creation failed: " << u_errorName(status);
        return input;
    }

    icu::Transliterator* translit = icu::Transliterator::createInstance("Any-Latin; Latin-ASCII", UTRANS_FORWARD, status);

    if (U_FAILURE(status)) {
        qWarning() << "Transliterator creation failed: " << u_errorName(status);
        delete translit;
        return input;
    }

    translit->transliterate(uInput);
    delete translit;

    std::string str_result;
    uInput.toUTF8String(str_result);

    return replaceEmojis(QString::fromStdString(str_result));
}


QString Transliterator::replaceEmojis(const QString &input) {
    const QMap<QString, QString> emojiTransliterator = {
        {"\xE2\x9D\xA4", "<3"},                   // ❤ heart
        {"\xE2\x98\xB9", ":-("},                  // ☹ frowning_face
        {"\xF0\x9F\x91\x8D", ":thumbs-up:"},      // 👍 thumbs-up
        {"\xF0\x9F\x91\x8E", ":thumbs-down:"},    // 👎 thumbs-down
        {"\xF0\x9F\x91\x8F", ":clap:"},           // 👏 clap
        {"\xF0\x9F\x94\xA5", ":fire:"},           // 🔥 fire
        {"\xF0\x9F\x99\x8F", ":praying:"},        // 🙏 praying
        {"\xF0\x9F\x98\x94", ":think:"},          // 😔 pensive
        {"\xF0\x9F\x91\x89", ":point:"},          // 👉 point
        {"\xF0\x9F\x91\x8B", ":wave:"},           // 👋 wave
        {"\xF0\x9F\x8E\x89", ":party:"},          // 🎉 party
        {"\xF0\x9F\x98\x8D", ":heart_eyes:"},     // 😍 heart_eyes
        {"\xF0\x9F\xA4\x94", ":thinking:"},       // 🤔 thinking
        {"\xF0\x9F\x98\x80", ":-D"},              // 😀 grinning
        {"\xF0\x9F\x98\x81", ":-D"},              // 😁 grinning_face_with_smiling_eyes
        {"\xF0\x9F\x98\x82", ":'D"},              // 😂 face_with_tears_of_joy
        {"\xF0\x9F\x98\x83", ":-D"},              // 😃 smiling_face_with_open_mouth
        {"\xF0\x9F\x98\x84", ":-D"},              // 😄 smiling_face_with_open_mouth_and_smiling_eyes
        {"\xF0\x9F\x98\x85", ":'D"},              // 😅 smiling_face_with_open_mouth_and_cold_sweat
        {"\xF0\x9F\x98\x86", "X-D"},              // 😆 smiling_face_with_open_mouth_and_tightly-closed_eyes
        {"\xF0\x9F\x98\x87", "O:-)"},             // 😇 innocent
        {"\xF0\x9F\x98\x89", ";-)"},              // 😉 wink
        {"\xF0\x9F\x98\x8A", ":-)"},              // 😊 blush
        {"\xF0\x9F\x98\x8B", ":-p"},              // 😋 yum
        {"\xF0\x9F\x98\x8E", "B-)"},              // 😎 sunglasses
        {"\xF0\x9F\x98\x95", ":-/"},              // 😕 confused
        {"\xF0\x9F\x98\x96", ":-S"},              // 😖 confounded_face
        {"\xF0\x9F\x98\x97", ":*"},               // 😗 kissing_face
        {"\xF0\x9F\x98\x98", ";-*"},              // 😘 face_throwing_a_kiss
        {"\xF0\x9F\x98\x99", ":-*"},              // 😙 kissing_face_with_smiling_eyes
        {"\xF0\x9F\x98\x9A", ":-*"},              // 😚 kissing_closed_eyes
        {"\xF0\x9F\x98\x9B", ":-P"},              // 😛 stuck_out_tongue
        {"\xF0\x9F\x98\x9C", ";-P"},              // 😜 stuck_out_tongue_winking_eye
        {"\xF0\x9F\x98\x9D", "X-P"},              // 😝 stuck_out_tongue_and_tightly-closed_eyes
        {"\xF0\x9F\x98\x9E", ":-S"},              // 😞 disappointed
        {"\xF0\x9F\x98\xA0", ":-@"},              // 😠 angry_face
        {"\xF0\x9F\x98\xA1", ":-@"},              // 😡 pouting_face
        {"\xF0\x9F\x98\xA2", ":'("},              // 😢 cry
        {"\xF0\x9F\x98\xA3", ":'("},              // 😣 persevering_face
        {"\xF0\x9F\x98\xA4", ":-O"},              // 😤 face_with_steam_from_nose
        {"\xF0\x9F\x98\xA5", ":'O"},              // 😥 face_with_cold_sweat
        {"\xF0\x9F\x98\xA8", ":-O"},              // 😨 fearful
        {"\xF0\x9F\x98\xA9", ":-O"},              // 😩 weary_face
        {"\xF0\x9F\x98\xAA", ":'("},              // 😪 sleepy_face
        {"\xF0\x9F\x98\xAB", ":-|"},              // 😫 tired_face
        {"\xF0\x9F\x98\xAC", ":-|"},              // 😬 grimacing_face
        {"\xF0\x9F\x98\xAD", ":'("},              // 😭 loudly_crying_face
        {"\xF0\x9F\x98\xAE", ":-O"},              // 😮 face_with_open_mouth
        {"\xF0\x9F\x98\xAF", ":-/"},              // 😯 hushed_face
        {"\xF0\x9F\x98\xB0", ":-/"},              // 😰 face_with_open_mouth_and_cold_sweat
        {"\xF0\x9F\x98\xB1", ":-O"},              // 😱 face_screaming_in_fear
        {"\xF0\x9F\x98\xB2", ":-/"},              // 😲 astonished_face
        {"\xF0\x9F\x98\xB3", ":-S"},              // 😳 flushed_face
        {"\xF0\x9F\x98\xB4", ":-S"},              // 😴 sleeping_face
        {"\xF0\x9F\x98\xB5", ":-P"},              // 😵 dizzy_face
        {"\xF0\x9F\x98\xB7", ":-x"},              // 😷 face_with_medical_mask
        {"\xF0\x9F\x98\xB8", "O.o"},              // 😸 grinning_cat_face_with_smiling_eyes
        {"\xF0\x9F\x98\xB9", ":-)"},              // 😹 cat_face_with_tears_of_joy
        {"\xF0\x9F\x98\xBA", ":-("},              // 😺 grinning_cat_face
        {"\xF0\x9F\x98\xBB", ":-D"},              // 😻 heart_eyes_cat
        {"\xF0\x9F\x98\xBC", ":'-("},             // 😼 cat_face_with_wry_smile
        {"\xF0\x9F\x98\xBD", ":'("},              // 😽 kissing_cat_face
        {"\xF0\x9F\x98\xBE", ":-@"},              // 😾 pouting_cat_face
        {"\xF0\x9F\x98\xBF", ":-@"},              // 😿 crying_cat_face
        {"\xF0\x9F\x99\x80", "O.O"},              // 🙀 screaming_cat_face
        {"\xF0\x9F\x99\x83", "(-:"},              // 🙃 upside-down_face
        {"\xF0\x9F\x91\x8C", ":ok:"},             // 👌 OK_hand
    };
    QString output(input);
    for (auto it = emojiTransliterator.begin(); it != emojiTransliterator.end(); ++it) {
        output.replace(it.key(), it.value());
    }
    return output;
}

void Transliterator::test() {

    QStringList inputStrings = {
        "The quick brown fox jumps over the lazy dog.", // English
        "Φάλαινα ψηλά στη ζουμπούλια ξεφτιλίζει κυρά Κλίτσα.", // Greek
        "Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich.", // German
        "Albert edward kyllikki soili jalmari viktor suomela si jönsson.", // Finnish
        "Frida var den skönaste kvinnan på Gotland.",           // Swedish
        "Příliš žluťoučký kůň úpěl ďábelské ódy", // Czech
        "Звонко чепуху щеголя прямо фьордом с экземплярами живых бегемотов.", // Russian
        "❤🙃😂😍👍🙏👏🔥👉🤔😁👌", // emojis
    };

    // Iterate through the input strings and transliterate each one
    foreach (const QString& input, inputStrings) {
        QString transliterated = Transliterator::convert(input);
        qDebug() << input
            << "\n >>>" << transliterated;
    }

}
