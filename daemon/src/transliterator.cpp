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

    return QString::fromStdString(str_result);
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
    };

    // Iterate through the input strings and transliterate each one
    foreach (const QString& input, inputStrings) {
        QString transliterated = Transliterator::convert(input);
        qDebug() << input
            << "\n >>>" << transliterated;
    }

}