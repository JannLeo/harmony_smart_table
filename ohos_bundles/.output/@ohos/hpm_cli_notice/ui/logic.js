const {
  dataApi: { ProjectApi },
  i18n,
  addonEvent,
  isDarkMode,
  ClientAddonApi,
  http
} = window.top.HPM_CLI_UI;

const NOTICE_NAME = 'Third Party Open Source Notice';

const render = (pluginName) => {
  const container = document.getElementById('pre');
  http.get(`/resource/${pluginName}/${NOTICE_NAME}`).then((data) => {
    if (data) {
      container.innerHTML = data;
    }
  }).catch((error) => {
    if (error && error.message && error.message.includes("no such file or directory")) {
      container.innerHTML = i18n.t(`${pluginName}.notFound`);
    } else {
      container.innerHTML = error && error.message;
    }
  });
  const htmlElm = document.getElementsByTagName('html')[0];
  isDarkMode ? htmlElm.classList.add('dark-mode') : htmlElm.classList.remove('dark-mode')
}

addonEvent.subscribe('themeChange', data => {
  document.getElementsByTagName('html')[0].className = data.isDarkMode ? 'dark-mode' : ''
}, true)

addonEvent.subscribe('addonLoaded', ({ name } = {}) => {
  render(name);
}, true)
