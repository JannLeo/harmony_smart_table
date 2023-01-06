(function(){
  const {
    ClientAddonApi,
    addonEvent
  } = window.top.HPM_CLI_UI;
  
  const NOTICE_NAME = 'Third Party Open Source Notice';
  
  const registI18nResource = (pluginName) => {
    const clientAddonApi = new ClientAddonApi();
    const zh_CN = {
      notice: "三方开源声明",
      notFound: `找不到对应的文件：${NOTICE_NAME}`
    }
  
    const en = {
      notice: "Notice",
      notFound: `Cannot found file: ${NOTICE_NAME}`
    }
  
    clientAddonApi.addLocalization('zh_CN', zh_CN, { pluginName });
    clientAddonApi.addLocalization('en', en, { pluginName });
  }

  addonEvent.subscribe('registered', name => {
    registI18nResource(name);
  })

})()
