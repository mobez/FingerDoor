
let main;
let title;
let get_v = false;
let b_jn;
let new_d=true;
let act_m=-1;

function ce(t) {
  return document.createElement(t);
};
function get_el(id){
	return document.getElementById(id);
}
function rem_int(){
	document.body.onkeydown = null;
}
function set_cl(id, pp, act=false) {
	get_el(id).onclick = jamp_p.bind(this, pp);
	if (act) document.body.onkeydown = function(e) {
	  if(e.keyCode == 13){
	    e.preventDefault();
	    jamp_p(pp);
	  };
	};
}
function set_cl_bk(el, st){
	try{
		const l =document.getElementsByClassName(el);
		for (let i=0; i<l.length; i++) {
		  l[i].style.display = st;
		};
	} catch (error) {
	  console.log(error.message);
	};
}
function del_el(el){
	try{
		const l =document.getElementsByClassName(el);
		for (let i=l.length; i>0; i--) {
		  l[i-1].remove();
		};
	} catch (error) {
	  console.log(error.message);
	};
}
function del_cl(el, cl){
	try{
		const l =document.getElementsByClassName(el);
		for (let i=l.length; i>0; i--) {
		  l[i-1].classList.remove(cl);
		};
	} catch (error) {
	  console.log(error.message);
	};
}
function set_el(el, wh=0, nm="", ht="", cl=""){
	e=ce((nm.length>0)?nm:"div");
	e.innerHTML=ht;
	switch (wh) {
		case 0:
			el.append(e);
			break;
		case 1:
			el.prepend(e);
		break;
		case 2:
			el.before(e);
		break;
		case 3:
			el.after(e);
		break;
		case 4:
			el.replaceWith(e);
		break;
	};
	if (cl.length > 1) e.classList.add("f_er");
	return e;
}
function set_err(tp){
	let err="", el, l=true;
	del_el("f_er");
	switch (tp) {
		case 0:
			el=get_el("i_l_n");
			if(el.value.length < 3){
				err = "Имя не может быть менее 3 символов";
				set_el(el, 3, "span", err, "f_er");
				l=false;
			};
			el=get_el("i_l_p");
			if(el.value.length < 8){
				err = "Пароль не может быть менее 8 символов";
				set_el(el, 3, "span", err, "f_er");
				l=false;
			};
			el=get_el("i_l_l");
			if(el.value.length < 3){
				err = "Логин не может быть менее 3 символов";
				set_el(el, 3, "span", err, "f_er");
				l=false;
			};
			el=get_el("i_l_p_lo");
			if(el.value.length < 4){
				err = "Пароль не может быть менее 4 символов";
				set_el(el, 3, "span", err, "f_er");
				l=false;
			};
			el=get_el("i_l_p_l");
			if(el.value.length < 4){
				err = "Пароль не может быть менее 4 символов";
				set_el(el, 3, "span", err, "f_er");
				l=false;
			};
		break;
		case 1:
			el=get_el("i_l_p_lo");
			err = "Неверный пароль!";
			set_el(el, 3, "span", err, "f_er");
	};
	return l;
}
function mactostr(mc){
	let res="", b;
	for (let i = 0; i < 6; i++) {
		b=mc[i].toString(16).toUpperCase();
		if (b.length == 1) b=0+b;
		res+=b;
		if (i < 5) res+=":";
	};
	return res;
}
async function send_val(pp){
	let fD = new FormData();
	try {
		switch(pp){
			case 1:

			break;
			case 2:
				if(set_err(0)){
					fD.append("mdns_name", get_el("i_l_n").value);
					fD.append("name", get_el("i_l_n_r").value);
					fD.append("pass", get_el("i_l_p_r").value);
					fD.append("pass_esp", get_el("i_l_p").value);
					fD.append("pass_old", get_el("i_l_p_lo").value);
					fD.append("pass_autch", get_el("i_l_p_l").value);
					fD.append("login", get_el("i_l_l").value);
					let wf = get_el("i_l_w");
					if (wf.value == 3){
						fD.append("route", true);
						fD.append("softap", true);
					}else if (wf.value == 2){
						fD.append("route", true);
						fD.append("softap", false);
					}else if (wf.value == 1){
						fD.append("route", false);
						fD.append("softap", true);
					}else{
						fD.append("route", false);
						fD.append("softap", false);
					}
					let res = await fetch('/set_lan', {
			      method: 'POST',
			      body: fD
			    });
			    if (res.status == 510){
			    	set_err(1);
			    	return false;
			    }
				}else{
					return false;
				}
			break;
			case 3:
				fD.append("channel", get_el("i_n_k").value);
				if (new_d){
					fD.append("ul_act", b_jn.ul_act);
					fD.append("mac_ul", mactostr(b_jn.mac_ul));
				}else{
					fD.append("ul_act", get_el("chb_u").checked);
					fD.append("mac_ul", get_el("i_n_m_u").value);
				}
				await fetch('/set_now', {
		      method: 'POST',
		      body: fD
		    });
			break;
			case 4:
				fD.append("rad_max", get_el("i_t_rm").value);
				fD.append("rad_delt", get_el("i_t_rd").value);
				fD.append("pult_max", get_el("i_t_pm").value);
				fD.append("pult_delt", get_el("i_t_pd").value);
				fD.append("ul_min", get_el("i_t_um").value);
				fD.append("ul_delt", get_el("i_t_ud").value);
				fD.append("ul_max", get_el("i_t_un").value);
				fD.append("ul_dop", get_el("i_t_up").value);
				await fetch('/set_temp', {
		      method: 'POST',
		      body: fD
		    });
			break;
			case 5:
				await fetch('/save_now', {
		      method: 'POST'
		    });
			break;
			default:
			break;
		};
	} catch (error) {
	  console.log(error.message);
	  return false;
	};
	return true;
}
function cltbl(jsn){
	console.log(jsn);
}
async function set_val(pp) {
	let vl;
	switch(pp){
		case 1:
			try {
				const res = await fetch("/lan.json");
				if (res.ok){
					const vl = await res.json();
					b_jn=vl;
					get_el("i_l_n").value = vl.mdns_name;
					get_el("i_l_p").value = vl.pass_esp;
					get_el("i_l_n_r").value = vl.name;
					get_el("i_l_p_r").value = vl.pass;
					get_el("i_l_l").value = vl.login;
					if (new_d) get_el("i_l_p_lo").value = vl.pass_autch;
					let wf = get_el("i_l_w");
					if(vl.route && vl.softap){
						wf.value = 3;
					}else if(vl.route){
						wf.value = 2;
					}else if(vl.softap){
						wf.value = 1;
					}else{
						wf.value = 0;
					};
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 2:
			try {
				const res = await fetch("/now.json");
				if (res.ok){
					const vl = await res.json();
					b_jn=vl;
					get_el("i_n_k").value = vl.channel;
					if (!new_d){
						get_el("chb_u").checked = vl.ul_act;
						if (vl.ul_act)
							get_el("ch_u").innerHTML = "Использовать";
						else
							get_el("ch_u").innerHTML = "Игнорировать";
						get_el("i_n_m_u").value = mactostr(vl.mac_ul);
					};
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 3:
			try {
				const res = await fetch("/temp.json");
				if (res.ok){
					const vl = await res.json();
					b_jn=vl;
					get_el("i_t_rm").value = vl.rad_max;
					get_el("i_t_rd").value = vl.rad_delt;
					get_el("i_t_pm").value = vl.pult_max;
					get_el("i_t_pd").value = vl.pult_delt;
					get_el("i_t_um").value = vl.ul_min;
					get_el("i_t_ud").value = vl.ul_delt;
					get_el("i_t_un").value = vl.ul_max;
					get_el("i_t_up").value = vl.ul_dop;
				}
			} catch (error) {
			  console.log(error.message);
			}
		break;
		case 4:
			try {
				const res = await fetch("/val");
				if (res.ok){
					const vl = await res.json();
					get_el("v_r").innerHTML = vl.rad.toFixed(1)+"°C";
					get_el("v_p").innerHTML = vl.p_temp.toFixed(1)+"°C";
					get_el("v_ph").innerHTML = parseInt(vl.p_hydr)+"%";
					get_el("v_u").innerHTML = vl.ul_temp.toFixed(1)+"°C";
					get_el("v_uh").innerHTML = parseInt(vl.ul_hydr)+"%";
					get_el("v_rs").innerHTML = vl.pwr?"Включен":"Отключен";
					if (!vl.ul_ok) set_cl_bk("ut", "none");
					else set_cl_bk("ut", "flex");
					if (!vl.p_ok) set_cl_bk("pt", "none");
					else set_cl_bk("pt", "flex");
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 5:
			const srch = await fetch("/search_ap");
				if (srch.ok){
					setTimeout(function(){
						fetch("/grt_ap").then(res=>res.json())
						.then(jsn=>cltbl(jsn));
					},1500);
				};
		break;
	};
}
async function add_ht(pg, nd){
	const res = await fetch("/"+pg);
	if (res.ok){
		const txt=await res.text();
		nd.insertAdjacentHTML('beforeend', txt);
	};
}
function check_s(e){
	switch (e.value) {
		case "1":
			if (e.checked)
				get_el("ch_u").innerHTML = "Использовать";
			else
				get_el("ch_u").innerHTML = "Игнорировать";
			break;
	};
}
async function go_pg(pg, frm = "", pp = 0) {
	try {
	const ok = await send_val(pp);
	if (ok) {
		rem_int();
		const res = await fetch("/"+pg);
			if (res.ok){
				main.innerHTML = await res.text();
				let cont;
				if (frm.length > 0){
					cont = await fetch("/"+frm);
				};
				if (res.ok){
					if (frm.length > 0){
						get_el("frm").innerHTML = await cont.text();
					};
					switch(pp){
						case 1:
							set_cl_bk("nw_h", "none");
							set_cl("btn_n", 5, true);
							set_val(1);
						break;
						case 2:
							set_cl("btn_p", 6);
							set_cl("btn_n", 7, true);
							set_val(2);
						break;
						case 3:
							set_cl("btn_p",8);
							set_cl("btn_n", 9, true);
							set_val(3);
							new_d=false;
						break;
						case 4:
						case 54:
							set_cl("btn_v", 10, true);
							set_val(4);
							get_el("btn_sts").classList.add("act");
						break;
						case 56:
							set_cl("btn_sv", 11, true);
							set_val(1);
							get_el("btn_lan").classList.add("act");
						break;
						case 57:
							set_cl("btn_sv", 12, true);
							set_val(3);
							get_el("btn_tmp").classList.add("act");
						break;
						case 58:
							set_cl("btn_sv", 13);
							get_el("btn_now").classList.add("act");
							add_ht("now2.ht", get_el("frm"));
							await set_val(2);
							get_el("chb_u").onclick=function(e){
								check_s(this);
							};
							get_el("i_n_m_us").onclick=function(e){
								get_el("pop").style.display="block";
							};
							get_el("popn").onclick = function(e){
								event.stopPropagation();
							};
							get_el("pop").onclick = function(e){
						  get_el("pop").style.display="none";
						  let tdl = get_el("ap");
					  	while (tdl.hasChildNodes()-1) {
							  tdl.removeChild(tdl.lastChild);
							};
						  };
						break;
					};
				};
			};
		};
	} catch (error) {
	  console.log(error.message);
	};
}

function jamp_p(pp) {
	del_cl("men-btn", "act");
	switch(pp){
		case 0:
			go_pg("val.ht", "vl.ht", 54);
		break;
		case 1:
			go_pg("lan_r.ht", "lan.ht", 56);
		break;
		case 2:
			go_pg("now_r.ht", "now1.ht", 58);
		break;
		case 3:
			go_pg("tmp_r.ht", "tmp.ht", 57);
		break;
		case 4:
			go_pg("start_1.ht", "lan.ht", 1);
		break;
		case 5:
			go_pg("start_2.ht", "now1.ht", 2);
		break;
		case 6:
			go_pg("start_1.ht", "lan.ht", 1);
		break;
		case 7:
			go_pg("start_3.ht", "tmp.ht", 3);
		break;
		case 8:
			go_pg("start_2.ht", "now1.ht", 2);
		break;
		case 9:
			go_pg("val.ht", "vl.ht", 4);
		break;
		case 10:
			set_val(4);
		break;
		case 11:
			send_val(2);
		break;
		case 12:
			send_val(4);
		break;
		case 13:
			send_val(3);
		break;
	};
}

async function init_p(json) {
	title = document.getElementById("ttl");
	main = document.getElementById("main");
	new_d = json.new_dev;
	if (new_d){
		const page = await fetch("/start.ht");
		main.innerHTML = await page.text();
		set_cl("btn_go", 4, true);
	}else{
		title.style.display = "block";
		go_pg("val.ht", "vl.ht", 54);
	};
}


function ready(){
	fetch("/now.json").then(res => res.json())
		.then(tt => init_p(tt));
	set_cl("btn_sts", 0);
	set_cl("btn_lan", 1);
	set_cl("btn_now", 2);
	set_cl("btn_tmp", 3);
}

document.addEventListener("DOMContentLoaded", ready);