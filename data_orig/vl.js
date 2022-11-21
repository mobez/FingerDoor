let main, timerId=null, sp_t,sp_h,sp_err;
const re_val = 30000;
const volh = "<div class=title_s><span class=txt_s>Значения</span></div><div class=form_l id=frm><div class=form_k><span class=\"err ev\"id=ev_err></span></div><div class=form_k><span class=\"f_s nv\">Температура:</span><span class=f_v id=ev_tmp></span></div><div class=form_k><span class=\"f_s nv\">Влажность:</span><span class=f_v id=ev_hud></span></div></div><div class=in_s><input class=btn_s id=btn_v type=button value=Обновить><input class=btn_s id=btn_od type=button value=Открыть></div>";

function ce(t) {
  return document.createElement(t);
};
function get_el(id){
	return document.getElementById(id);
}
function se(el, e){
	el.append(e);
}
async function up_vol() {
	try {
		fetch("/val").catch(()=>{
			sp_err.innerHTML = "Ошибка соединения";
			sp_err.classList.toggle("err",true);
			if (timerId){
				clearTimeout(timerId);
				timerId = null;
			}
			timerId = setTimeout(up_vol, re_val);
		}).then(res => res.json()).then(vl=>{
			if (!vl.tm){
				sp_err.innerHTML = "Время не синхронизировано!";
				sp_err.classList.toggle("err",true);
			}else{
				let time = vl.tm * 1000;
				const date = Number(new Date());
				let dt = new Date(time);
				if ((date-30000) > time){
					let ot = Math.floor((date - time)/60000);
					let he="";
					if (ot > 59){
						let ho = Math.floor(ot/60);
						let mo = ot%60;
						let strH="", strM="", sMM="";
						if ((ho < 2)||((ho % 100) > 20 && (ho % 10) < 2 && (ho % 10) > 0)) strH = " час";
						else if((ho < 5)||((ho % 100) > 20 && (ho % 10) < 5 && (ho % 10) > 0)) strH = " часа";
						else strH = " часов";

						if ((mo < 2)||(mo > 20 && (mo % 10) < 2 && (mo % 10) > 0)) strM = " минуту";
						else if((mo < 5)||(mo > 20 && ((mo % 10) < 5) && ((mo % 10) > 0))) strM = " минуты";
						else strM = " минут";
						if (mo>0)sMM=" "+mo+strM;
						he = ho+strH+sMM;
					}else{
						if ((ot < 2)||(ot > 20 && (ot % 10) < 2 && (ot % 10) > 0)) strM = " минуту";
						else if((ot < 5)||(ot > 20 && ((ot % 10) < 5) && ((ot % 10) > 0))) strM = " минуты";
						else strM = " минут";
						he = ot+strM+"!";
					}
					sp_err.innerHTML = "Время отстает на "+he+" ("+(dt.getHours()<10?"0":"")+dt.getHours()+":"+(dt.getMinutes()<10?"0":"")+dt.getMinutes()+")";
					sp_err.classList.toggle("err",true);
				}else{
					sp_err.innerHTML = "Время "+(dt.getHours()<10?"0":"")+dt.getHours()+":"+(dt.getMinutes()<10?"0":"")+dt.getMinutes();
					sp_err.classList.toggle("err",false);
				};
				sp_t.innerHTML = vl.vol.temp.toFixed(2)+"°C";
				sp_h.innerHTML = vl.vol.hud.toFixed(0)+"%";					
			};
			if (timerId){
				clearTimeout(timerId);
				timerId = null;
			}
		  timerId = setTimeout(up_vol, re_val);
		});
	} catch (error) {
	  console.log(error.message);
		if (timerId){
			clearTimeout(timerId);
			timerId = null;
		}
	  timerId = setTimeout(up_vol, re_val);
	};
}


function ready(){
	main = get_el("main");
	main.innerHTML = volh;
	get_el("btn_v").onclick = up_vol;
	get_el("btn_od").onclick = () => {
		fetch("/open", {
			method: "PUT"
		}).then(res => res.text()).then(txt=>{alert(txt);});
	};
	sp_err = document.getElementById("ev_err");
	sp_t = document.getElementById("ev_tmp");
	sp_h = document.getElementById("ev_hud");
	up_vol();
}

document.addEventListener("DOMContentLoaded", ready);