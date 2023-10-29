#ifndef TRANSLITERATOR_H
#define TRANSLITERATOR_H

#include <QString>

class Transliterator {
public:
    static QString convert(const QString& input);
    static QString replaceEmojis(const QString& input);
    static void test();
};

#endif // TRANSLITERATOR_H