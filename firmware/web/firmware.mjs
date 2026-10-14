export function validateFirmware(file,m){
 if(!file||!m||m.hardware!=='esp32c3-drv8871'||m.settingsSchema!==1)return 'Firmware target or settings version is incompatible';
 if(!Number.isInteger(m.length)||m.length<288||m.length>1966080||file.size!==m.length)return 'Firmware file size does not match its manifest';
 if(typeof m.version!=='string'||!m.version||m.version.length>32||!(/^[a-f0-9]{64}$/).test(m.sha256))return 'Invalid firmware manifest';
 if(!file.name.endsWith('.bin'))return 'Choose the application .bin file';return null;
}
export async function firmwareUpload(file,manifest,transport,onProgress=()=>{}){
 const error=validateFirmware(file,manifest);if(error)throw Error(error);
 const prepared=await transport.prepare(manifest);if(!prepared.ok)throw Error(prepared.error||'Update preparation rejected');
 try{await transport.ready();const result=await transport.upload(file,prepared.uploadId,onProgress);if(!result.ok)throw Error(result.error||'Firmware verification failed');return result;}
 catch(e){try{await transport.cancel();}catch{}throw e;}
}
