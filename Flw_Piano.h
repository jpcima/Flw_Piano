//          Copyright Jean Pierre Cimalando 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#if !defined(FLW_PIANO_H)
#define FLW_PIANO_H

#include <FL/Fl_Group.H>
#include <memory>

#if __cplusplus < 201103L && !defined(override)
#define override
#endif

class Flw_Piano : public Fl_Group {
public:
    Flw_Piano(int x, int y, int w, int h);
    ~Flw_Piano();

    const Fl_Color *key_color() const;
    void key_color(Fl_Color wh, Fl_Color bl);

    const Fl_Boxtype *key_box() const;
    void key_box(Fl_Boxtype wh, Fl_Boxtype bl);

    unsigned key_count() const;
    void key_count(unsigned nkeys);

    int key_value(unsigned key) const;
    void key_value(unsigned key, int value);

    bool press_key(unsigned key);
    bool release_key(unsigned key);

    void draw() override;

public:
    struct Impl;
private:
#if __cplusplus >= 201103L
    const std::unique_ptr<Impl> P;
#else
    const std::auto_ptr<Impl> P;
#endif
};

enum Piano_Event {
    PIANO_PRESS,
    PIANO_RELEASE,
};

namespace Piano {
    Piano_Event event();
    unsigned key();
}

#endif  // !defined(FLW_PIANO_H)
