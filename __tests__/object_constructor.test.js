const { load, Gtk } = require('../');

const GLib = load('GLib');
const GObject = load('GObject');

describe('Object constructor', function() {
    describe('with "new" operator', function() {
        it('without arguments', function() {
            const label = new Gtk.Label();
            expect(label.label).toEqual('');
        });

        it('with properties', function() {
            const label = new Gtk.Label({ label: 'aText99'});
            expect(label.label).toEqual('aText99');
        });
    });

    describe('using static functions', function() {
        /* FIXME: pending. Calling .new() ends up calling GObject.Object.new() directly and not
         * the .new() of Gtk.Button. Ordering problem in registration of static functions? */
        it('new()', null, function() {
            const button = Gtk.Button.new();
            expect(button.label).toEqual(null);
        });

        it('newWithArgument(arg)', function() {
            const button = Gtk.Button.newWithLabel('myLabel312');
            expect(button.label).toEqual('myLabel312');
        });
    });

    describe('with properties', function() {
        it('derived from interface', function() {
            const box = new Gtk.Box({ orientation : Gtk.Orientation.vertical });
            expect(box.orientation).toEqual(Gtk.Orientation.vertical);
        });
    });
});

