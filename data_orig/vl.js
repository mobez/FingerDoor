let main, timerId=null, stat_jn, sp_d=[],sp_err;
const re_val = 30000;
const volh = "<div class=title_s><span class=txt_s>Значения</span></div><div class=form_l id=frm></div><div class=in_s><input class=btn_s id=btn_v type=button value=Обновить></div>";

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
			let len_v = vl.vol.length<stat_jn.devs.length?vl.vol.length:stat_jn.devs.length;
			if (!vl.tm){
				sp_err.innerHTML = "Данные еще не поступали!";
				sp_err.classList.toggle("err",true);
			}else{
				let time = vl.tm * 1000;
				const date = Number(new Date());
				if ((date-900000) > time){
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
						else if((mo < 5)||(mo > 20 && (mo % 10) < 5 && (mo % 10) > 0)) strM = " минуты";
						else strM = " минут";
						if (mo>0)sMM=" "+mo+strM;
						he = ho+strH+sMM;
					}else{
						he = ot+" минут!";
					}
					sp_err.innerHTML = "Данные не поступают "+he;
					sp_err.classList.toggle("err",true);
				}else{
					let dt = new Date(time);
					sp_err.innerHTML = "Данные за "+(dt.getHours()<10?"0":"")+dt.getHours()+":"+(dt.getMinutes()<10?"0":"")+dt.getMinutes();
					sp_err.classList.toggle("err",false);
				};
				for (let i = 0; i < len_v; i++) {
					if (stat_jn.devs[i].act){
						sp_d[i].innerHTML=vl.vol[i].toFixed(stat_jn.devs[i].dec)+stat_jn.devs[i].v_name;
					};
				};						
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
	fetch("/station.json").then(res => res.json())
	.then(tt => {
		stat_jn=tt;
		let frm_v = get_el("frm");
		let div_v, sn_v_n, sn_v_v;
		div_v=ce("div");
		div_v.classList.add("form_k");
		sp_err=ce("span");
		sp_err.classList.add("ev");
		se(frm_v,div_v);
		se(div_v,sp_err);
		for (var i = 0; i < stat_jn.devs.length; i++) {
			if (stat_jn.devs[i].act){
				div_v=ce("div");
				div_v.classList.add("form_k");
				sn_v_n=ce("span");
				sn_v_n.innerHTML=stat_jn.devs[i].name+":";
				sn_v_n.classList.add("f_s");
				sn_v_n.classList.add("nv");
				sn_v_v=ce("span");
				sn_v_v.innerHTML="0"+stat_jn.devs[i].v_name;
				sn_v_v.classList.add("f_v");
				sp_d[i]=sn_v_v;
				se(frm_v,div_v);
				se(div_v,sn_v_n);
				se(div_v,sn_v_v);
			};
		};
		up_vol();
	});
}

document.addEventListener("DOMContentLoaded", ready);