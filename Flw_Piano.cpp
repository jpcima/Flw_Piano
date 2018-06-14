//          Copyright Jean Pierre Cimalando 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Flw_Piano.h"
#include <FL/Fl_Button.H>
#include <FL/Fl.H>
#include <vector>
#include <math.h>

enum Key_Color { Key_White, Key_Black };
enum { white_keywidth = 24, black_keywidth = 14 };

struct Flw_Piano::Impl {
    explicit Impl(Flw_Piano *q);
    Flw_Piano *const Q;

    Fl_Boxtype box_[2];
    Fl_Color bg_[2];

    class Key_Button : public Fl_Button {
    public:
        Key_Button(int x, int y, int w, int h, unsigned key)
            : Fl_Button(x, y, w, h), key_(key) {}
        unsigned piano_key() const
            { return key_; }
        int handle(int event) override;
    private:
        unsigned key_;
    };

    std::vector<Key_Button *> keys_;
    unsigned pushed_key_;
    static int handle_key(Key_Button *btn, int event);

    void create_keys(unsigned nkeys);
    void delete_keys();
    Key_Button *key_at(int x, int y);

    static const int *keypos_;
    static void make_positions(int *pos);

    static Key_Color key_color(unsigned key);
    static unsigned next_white_key(unsigned key);
    static int key_position(unsigned key);
    static int key_width(unsigned key);

    static Piano_Event event_type_;
    static unsigned event_key_;
    void do_piano_callback(Piano_Event type, unsigned key);
};

Flw_Piano::Flw_Piano(int x, int y, int w, int h)
    : Fl_Group(x, y, w, h),
      P(new Impl(this))
{
    if (!Impl::keypos_) {
        static int table[12];
        Impl::make_positions(table);
        Impl::keypos_ = table;
    }
    key_count(48);
}

Flw_Piano::~Flw_Piano()
{
}

const Fl_Color *Flw_Piano::key_color() const
{
    return P->bg_;
}

void Flw_Piano::key_color(Fl_Color wh, Fl_Color bl)
{
    P->bg_[0] = wh;
    P->bg_[1] = bl;

    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();

    for (unsigned key = 0; key < nkeys; ++key) {
        Key_Color keyc = P->key_color(key);
        Fl_Color bg = (keyc == Key_White) ? wh : bl;
        keys[key]->color(bg, bg);
    }
}

const Fl_Boxtype *Flw_Piano::key_box() const
{
    return P->box_;
}

void Flw_Piano::key_box(Fl_Boxtype wh, Fl_Boxtype bl)
{
    P->box_[0] = wh;
    P->box_[1] = bl;

    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();

    for (unsigned key = 0; key < nkeys; ++key) {
        Key_Color keyc = Impl::key_color(key);
        keys[key]->box((keyc == Key_White) ? wh : bl);
    }
}

unsigned Flw_Piano::key_count() const
{
    return P->keys_.size();
}

void Flw_Piano::key_count(unsigned nkeys)
{
    P->create_keys(nkeys);
}

int Flw_Piano::key_value(unsigned key) const
{
    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();
    if (key >= nkeys)
        return 0;
    return keys[key]->value();
}

void Flw_Piano::key_value(unsigned key, int value)
{
    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();
    if (key >= nkeys)
        return;
    keys[key]->value(value);
}

bool Flw_Piano::press_key(unsigned key)
{
    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();
    if (key >= nkeys)
        return false;
    Impl::Key_Button *btn = keys[key];
    if (btn->value())
        return false;
    btn->value(1);
    P->do_piano_callback(PIANO_PRESS, key);
    return true;
}

bool Flw_Piano::release_key(unsigned key)
{
    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();
    if (key >= nkeys)
        return false;
    Impl::Key_Button *btn = keys[key];
    if (!btn->value())
        return false;
    btn->value(0);
    P->do_piano_callback(PIANO_RELEASE, key);
    return true;
}

void Flw_Piano::draw()
{
    Impl::Key_Button **keys = P->keys_.data();
    unsigned nkeys = P->keys_.size();

    for (unsigned key = 0; key < nkeys; ++key) {
        Key_Color keyc = P->key_color(key);
        if (keyc == Key_White)
            draw_child(*keys[key]);
    }
    for (unsigned key = 0; key < nkeys; ++key) {
        Key_Color keyc = P->key_color(key);
        if (keyc == Key_Black)
            draw_child(*keys[key]);
    }
}

Flw_Piano::Impl::Impl(Flw_Piano *q)
    : Q(q),
      pushed_key_((unsigned)-1)
{
    box_[0] = FL_UP_BOX;
    box_[1] = FL_UP_BOX;
    bg_[0] = fl_rgb_color(0xee, 0xee, 0xec);
    bg_[1] = fl_rgb_color(0x88, 0x8a, 0x85);
}

int Flw_Piano::Impl::Key_Button::handle(int event)
{
    if (static_cast<Flw_Piano::Impl *>(user_data())->handle_key(this, event))
        return 1;
    return Fl_Button::handle(event);
}

int Flw_Piano::Impl::handle_key(Key_Button *btn, int event)
{
    Impl *self = static_cast<Impl *>(btn->user_data());
    Flw_Piano *Q = self->Q;
    Key_Button **keys = self->keys_.data();

    switch (event) {
    case FL_PUSH:
    case FL_DRAG: {
        int x = Fl::event_x();
        int y = Fl::event_y();
        Key_Button *btn = self->key_at(x, y);
        if (btn) {
            unsigned key = btn->piano_key();
            unsigned oldkey = self->pushed_key_;
            if (key != oldkey) {
                Q->release_key(oldkey);
                Q->press_key(key);
                self->pushed_key_ = key;
            }
        }
        return 1;
    }
    case FL_RELEASE: {
        unsigned key = self->pushed_key_;
        Q->release_key(key);
        self->pushed_key_ = (unsigned)-1;
        return 1;
    }
    }

    return 0;
}

void Flw_Piano::Impl::create_keys(unsigned nkeys)
{
    delete_keys();
    if (nkeys == 0)
        return;

    const Fl_Boxtype *box = box_;
    Fl_Color bg_white = bg_[0];
    Fl_Color bg_black = bg_[1];

    int x = Q->x();
    int y = Q->y();
    int w = Q->w();
    int h = Q->h();

    int fullw = key_position(nkeys - 1) + key_width(nkeys - 1);
    double wr = (double)w / (double)fullw;

    keys_.resize(nkeys);
    for (unsigned key = 0; key < nkeys; ++key) {
        Key_Color keyc = key_color(key);
        if (keyc == Key_White) {
            int keyx = x + lround(wr * key_position(key));
            unsigned nextkey = next_white_key(key);
            int nextx = x + lround(wr * (keypos_[nextkey % 12] + (int)(nextkey / 12) * 7 * keypos_[2]));
            int keyw = nextx - keyx;
            int keyh = h;
            Key_Button *btn = new Key_Button(keyx, y, keyw, keyh, key);
            btn->user_data(this);
            btn->visible_focus(0);
            btn->color(bg_white, bg_white);
            btn->box(box[0]);
            keys_[key] = btn;
        }
    }
    for (unsigned key = 0; key < nkeys; ++key) {
        Key_Color keyc = key_color(key);
        if (keyc == Key_Black) {
            int keyx = x + lround(wr * key_position(key));
            int keyw = lround(wr * black_keywidth);
            int keyh = h / 2;
            Key_Button *btn = new Key_Button(keyx, y, keyw, keyh, key);
            btn->user_data(this);
            btn->visible_focus(0);
            btn->color(bg_black, bg_black);
            btn->box(box[1]);
            keys_[key] = btn;
        }
    }
}

void Flw_Piano::Impl::delete_keys()
{
    while (!keys_.empty()) {
        delete keys_.back();
        keys_.pop_back();
    }
}

Flw_Piano::Impl::Key_Button *Flw_Piano::Impl::key_at(int x, int y)
{
    Key_Button **keys = keys_.data();
    unsigned nkeys = keys_.size();

    for (unsigned i = 0; i < nkeys; ++i) {
        if (key_color(i) == Key_Black) {
            Key_Button *btn = keys[i];
            bool inside = x >= btn->x() && x < btn->x() + btn->w() &&
                y >= btn->y() && y < btn->y() + btn->h();
            if (inside)
                return btn;
        }
    }
    for (unsigned i = 0; i < nkeys; ++i) {
        if (key_color(i) == Key_White) {
            Key_Button *btn = keys[i];
            bool inside = x >= btn->x() && x < btn->x() + btn->w() &&
                y >= btn->y() && y < btn->y() + btn->h();
            if (inside)
                return btn;
        }
    }

    return NULL;
}

Key_Color Flw_Piano::Impl::key_color(unsigned key)
{
    unsigned n = key % 12;
    n = (n < 5) ? n : (n - 1);
    return (n & 1) ? Key_Black : Key_White;
}

unsigned Flw_Piano::Impl::next_white_key(unsigned key)
{
    unsigned nextkey = key + 1;
    while (key_color(nextkey) != Key_White)
        ++nextkey;
    return nextkey;
}

int Flw_Piano::Impl::key_position(unsigned key)
{
    return keypos_[key % 12] + (int)(key / 12) * 7 * keypos_[2];
}

int Flw_Piano::Impl::key_width(unsigned key)
{
    if (key_color(key) == Key_White)
        return key_position(next_white_key(key)) - key_position(key);
    else
        return black_keywidth;
}

const int *Flw_Piano::Impl::keypos_ = NULL;

void Flw_Piano::Impl::make_positions(int *pos)
{
    unsigned nth = 0;

    for (int i = 0; i < 7; ++i) {
        int index = i * 2;
        index -= (index > 4) ? 1 : 0;
        pos[index] = i * white_keywidth;
    }

    for (int i = 0; i < 2; ++i)
        pos[1 + 2 * i] = 15 + 2 * i * black_keywidth;
    for (int i = 0; i < 3; ++i)
        pos[6 + 2 * i] = pos[5] + 13 + 2 * i * black_keywidth;
}

Piano_Event Flw_Piano::Impl::event_type_;
unsigned Flw_Piano::Impl::event_key_;

void Flw_Piano::Impl::do_piano_callback(Piano_Event type, unsigned key)
{
    event_type_ = type;
    event_key_ = key;
    Q->do_callback();
}

Piano_Event Piano::event()
{
    return Flw_Piano::Impl::event_type_;
}

unsigned Piano::key()
{
    return Flw_Piano::Impl::event_key_;
}
