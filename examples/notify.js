// docs: http://developer.gnome.org/libnotify/0.7/NotifyNotificationotification.html
const { load } = require('../');

const notify = load('Notify');

notify.init('notify_test.js sample application');

function sleep(milliseconds) {
  return new Promise(resolve => setTimeout(() => resolve(), milliseconds));
}

async function main() {
  const notification = new notify.Notification({ summary: 'a' });
  notification.update(
    'Notify Test',
    'This is a test notification message via Node.JS.'
  );
  notification.show();
  await sleep(2000);

  notification.update('Notification Update', 'The status has been updated!');
  notification.show();
  await sleep(2000);

  notification.update('Ethan', 'This message will self-destruct in 2 seconds.');
  notification.show();
  await sleep(2000);

  notification.close();
}

main();
