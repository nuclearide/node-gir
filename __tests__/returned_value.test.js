const { load, Gtk } = require('../');

const GObject = load('GObject');
const GdkPixbuf = load('GdkPixbuf');

const win = new Gtk.Window({
    type: Gtk.WindowType.toplevel,
    title: 'Node.JS GTK Window'
});

const pixbuf = new GdkPixbuf.Pixbuf(0, false, 1, 1, 1);

describe('Return value', function() {
    it('number', function() {
        const intValue = GObject.typeFromName('GtkWindow');
        expect(typeof(intValue)).toEqual('number');
    });

    it('char', function() {
        win.title = 'Lancelot';
        expect(win.title).toEqual('Lancelot');
        expect(typeof(win.title)).toEqual('string');
    });

    it('boolean', function() {
        const decorated = win.getDecorated();
        expect(typeof(decorated)).toEqual('boolean');
    });

    it('null', function() {
        win.setIcon(null);
        expect(win.getIcon()).toBeNull();
    });

    it('object', function() {
        win.icon = pixbuf;
        expect(typeof(win.icon)).toEqual('object');
    });

    it('void', function() {
        const voidValue = win.resize(10, 10);
        expect(voidValue).toBeUndefined();
    });

    describe('array', function() {
        it('GType', function() {
            const gtype = GObject.typeFromName('GtkWindow');
            const gtypeArray = GObject.typeChildren(gtype);
            expect(gtypeArray.length).not.toEqual(0);
        });
    });
});
