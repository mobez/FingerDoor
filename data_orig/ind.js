
let main;
let title;
let get_v = false;
let b_jn, id_p, timerId=null;
let act_m=-1;
let stat_jn,sp_t,sp_h,sp_err;
let servers=[];
let devs=[];
let con_lan={};
let serv_id=-1;
let dev_id=0;
let now_j, act_j;
let peer_add={};
const phalanxs = ["Большой", "Средний", "Указательный", "Безымянный", "Мизинец"];
const volh = "<div class=title_s><span class=txt_s>Значения</span></div><div class=form_l id=frm><div class=form_k><span class=\"err ev\"id=ev_err></span></div><div class=form_k><span class=\"f_s nv\">Температура:</span><span class=f_v id=ev_tmp></span></div><div class=form_k><span class=\"f_s nv\">Влажность:</span><span class=f_v id=ev_hud></span></div></div><div class=in_s><input class=btn_s id=btn_v type=button value=Обновить><input class=btn_s id=btn_od type=button value=Открыть></div><div class=in_s><input class=btn_s id=btn_res type=button value=Перезагрузить> <input class=inp_hid id=fl_i type=file accept=.json name=fl_i onchange=sh_cnf(this)><label class=\"btn_s pr\"for=fl_i><span>Применить конф</span></label> <input class=btn_s id=btn_gcnf type=button value=\"Скачать конф\"></div>";
const lan = "<div class=form_k><span class=f_s>Wifi</span> <select class=\"f_i t_s\"id=i_l_w type=text><option value=1>Точка доступа<option value=2>Клиент<option value=3>Точка доступа и клиент</select></div><div class=form_k><span class=f_s>Имя</span> <input class=\"f_i t_s\"id=i_l_n placeholder=Имя></div><div class=form_k><span class=f_s>Пароль</span> <input class=\"f_i t_s\"id=i_l_p placeholder=Пароль type=password></div><div class=form_k><span class=f_s>Точка доступа</span> <input class=\"f_i t_s\"id=i_l_n_r placeholder=\"Точка доступа(Роутер)\"></div><div class=form_k><span class=f_s>Пароль точки</span> <input class=\"f_i t_s\"id=i_l_p_r placeholder=\"Пароль точки\"type=password></div><div class=form_k><span class=f_s>Логин</span> <input class=\"f_i t_s\"id=i_l_l placeholder=\"Точка доступа\"title=\"Доступ по web\"></div><div class=\"form_k nw_h\"><span class=f_s>Старый пароль</span> <input class=\"f_i t_s\"id=i_l_p_lo placeholder=Пароль type=password></div><div class=form_k><span class=f_s>Пароль</span> <input class=\"f_i t_s\"id=i_l_p_l placeholder=Пароль type=password></div>";
const lan_r = "<div class=title_s><span class=txt_s>Настройка сети</span></div><div class=form_l id=frm></div><div class=in_s><input class=btn_s id=btn_sv type=button value=Сохранить></div>";
const now_r = "<div class=title_s><span class=txt_s>Настройка ESP-NOW</span></div><div class=form_l id=frm></div><div class=in_s><input class=btn_s id=btn_sv type=button value=Сохранить></div><div class=title_s><span class=txt_s>Активные пиры</span></div><div class=form_l id=frmp_a><table class=peer id=tbp_a><tbody id=peer_act><tr><th>MAC<th></table></div><div class=title_s><span class=txt_s>Настройка пиров</span></div><div class=form_l id=frmp><table class=peer id=tbp><tbody id=peer><tr><th>Имя<th>MAC<th>Удалить<th>Настроить</table><div class=in_s><input class=\"t_s btn_a\"id=btn_avp type=button value=Добавить></div></div><div class=ovrl id=pop><div class=pop id=popn><fieldset class=fiel><legend class=lgndf id=lgn>Точки доступа</legend><table class=ap id=tbl><tbody id=ap><tr><th>Имя<th>Сигнал<th>MAC<th>Канал<th>Hide</table></fieldset><div class=in_s><input class=btn_s id=btn_ps type=button value=Сканировать></div></div></div><div class=ovrl id=pop2><div class=pop id=popn2><fieldset class=fiel><legend class=lgndf id=lgn_r>Настрайка для приема</legend><div class=form_k><span class=f_s>Тип</span> <span class=\"f_i t_s\"id=i_n_n_p></span></div><div class=form_k><span class=f_s>Наименование</span> <input class=\"f_i t_s\"id=i_n_l_p placeholder=Наименование></div><div class=form_k><span class=f_s>Начальный id изм</span> <select class=\"f_i t_s\"id=i_n_i_p type=text></select></div><div class=form_k><span class=f_s>Кол-во изм</span> <select class=\"f_i t_s\"id=i_n_c_p type=text></select></div><div class=form_k><span class=f_s>Автоизмерение</span> <input class=chb id=chb_s_a type=checkbox> <label class=\"f_i ch\"for=chb_s_a id=ch_p_m></label></div></fieldset><div class=in_s><input class=btn_s id=btn_psp type=button value=Сохранить></div><fieldset class=fiel><legend class=lgndf id=lgn_r>Настрайка устройства</legend><div class=form_k><span class=f_s>Wifi</span> <input class=chb id=chb_s_w type=checkbox value=2> <label class=\"f_i ch\"for=chb_s_w id=ch_p_w></label></div><div class=form_k><span class=f_s>Bluetooth</span> <input class=chb id=chb_s_b type=checkbox value=3> <label class=\"f_i ch\"for=chb_s_b id=ch_p_b></label></div></fieldset><div class=in_s><input class=btn_s id=btn_psn type=button value=Сохранить> <input class=btn_s id=btn_prn type=button value=Reboot></div><div class=title_s><span class=txt_s>Пиры устройства</span></div><table class=peer id=tbp_p><tbody id=peer_pdev><tr><th>MAC<th>Channel<th></table><div class=in_s><input class=btn_s id=btn_clr type=button value=Очистить></div></div></div>";
const now_1 = "<div class=form_k><span class=f_s>Канал</span> <select class=\"f_i t_s\"id=i_n_k type=text><option value=1>1<option value=2>2<option value=3>3<option value=4>4<option value=5>5<option value=6>6<option value=7>7<option value=8>8<option value=9>9<option value=10>10<option value=11>11<option value=12>12<option value=13>13</select></div>";
const timh = "<div class=form_k><span class=f_s>Сервер времени 1</span> <input class=\"f_i t_s\"id=i_t_s1 placeholder=0.pool.ntp.org></div><div class=form_k><span class=f_s>Сервер времени 2</span> <input class=\"f_i t_s\"id=i_t_s2 placeholder=1.pool.ntp.org></div><div class=form_k><span class=f_s>Сервер времени 3</span> <input class=\"f_i t_s\"id=i_t_s3 placeholder=2.pool.ntp.org></div><div class=form_k><span class=f_s>Переод отправки измерений</span> <input class=\"f_i t_s\"id=i_t_pt placeholder=\"Переод отправки измерений\"title=\"в минутах\"step=1 type=number></div>";
const tim_r = "<div class=title_s><span class=txt_s>Настройка времени</span></div><div class=form_l id=frm></div><div class=in_s><input class=btn_s id=btn_sv type=button value=Сохранить></div>";
const serv_r = "<div class=title_s><span class=txt_s>Настройка сервера приема</span></div><div class=form_l id=frm></div><div class=in_s><input class=btn_s id=btn_sv type=button value=Сохранить></div>";
const servh = "<div class=t_pl><div class=cnt_t><table class=ap id=el_t><tr><th>Серверы</table></div><div class=cnf_t><span class=txt_t id=n_tt></span><div class=form_k><span class=f_s>Использовать</span> <input class=chb id=chb_s type=checkbox value=1> <label class=\"f_i ch\"for=chb_s id=ch_s></label></div><div class=form_k><span class=f_s>Хост </span><input class=\"f_i t_s\"id=i_s_ph placeholder=\"Адрес сервера\"></div><div class=form_k><span class=f_s>Порт </span><input class=\"f_i t_s\"id=i_s_pp placeholder=Порт step=1 type=number></div><input class=btn_s id=btn_spv type=button value=Применить></div></div>";
const stan_r = "<div class=title_s><span class=txt_s>Настройка отпечатков</span></div><div class=form_l id=frm></div><div class=in_s><input class=btn_s id=btn_sad type=button value=Добавить><input class=btn_s id=btn_srem type=button value=Очистить><input class=btn_s id=btn_sv type=button value=Сохранить></div>";
const stanh = "<div class=t_pl><div class=cnt_t><table class=ap id=el_t><tr><th>Отпечатки</table></div><div class=cnf_t><span class=txt_t id=n_tt></span><div class=form_k><span class=f_s>Использовать</span> <input class=chb id=chb_e type=checkbox value=1> <label class=\"f_i ch\"for=chb_e id=ch_e></label></div><div class=form_k><span class=f_s>Наименование </span><input class=\"f_i t_s\"id=i_e_n placeholder=\"Наименование элемента\"></div><div class=form_k><span class=f_s></div><div class=form_k><span class=f_s>Палец</span> <select class=\"f_i t_s\"id=i_n_d type=text><option value=0>Большой палец<option value=1>Указательный палец<option value=2>Средний палец<option value=3>Безымянный палец<option value=4>Мизинец</select></div><input class=btn_s id=btn_spv type=button value=Применить><input class=btn_s id=btn_spr type=button value=Удалить></div></div><div class=ovrl id=pop2><div class=pop id=popn2><fieldset class=fiel><legend class=lgndf id=lgn_r>Добавление пальца</legend><div class=form_k><span class=f_s>Имя</span> <input class=\"f_i t_s\"id=i_n_l_p placeholder=Имя></div><div class=form_k><span class=f_s>Палец</span> <select class=\"f_i t_s\"id=i_n_i_p type=text></select></div></fieldset><div class=in_s><input class=btn_s id=btn_ssp type=button value=Добавить></div></div></div>";
const re_val = 30000;
const max_re_sts=30;


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
function se(el, wh=0, e){
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
}
function set_el(el, wh=0, nm="", ht="", cl=""){
	e=ce((nm.length>0)?nm:"div");
	e.innerHTML=ht;
	se(el, wh, e);
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

function download_conf(){
	fetch("/cnf_get")
    .then(res => res.blob())
    .then(blob => {
        var url = window.URL.createObjectURL(blob);
        var a = document.createElement('a');
        a.href = url;
        a.download = "Config.json";
        document.body.appendChild(a);
        a.click();
        a.remove();
    });
}
async function send_val(pp, alrt = true, cb=null){
	let snd = false;
	let fD = new FormData();
	try {
		switch(pp){
			case 2:
				if(set_err(0)){
					con_lan.mdns_name = get_el("i_l_n").value;
					con_lan.name = get_el("i_l_n_r").value;
					con_lan.pass = get_el("i_l_p_r").value;
					con_lan.pass_esp = get_el("i_l_p").value;
					con_lan.pass_old = get_el("i_l_p_lo").value;
					con_lan.pass_autch = get_el("i_l_p_l").value;
					con_lan.login = get_el("i_l_l").value;
					let wf = get_el("i_l_w");
					if (wf.value == 3){
						con_lan.route = true;
						con_lan.softap = true;
					}else if (wf.value == 2){
						con_lan.route = true;
						con_lan.softap = false;
					}else if (wf.value == 1){
						con_lan.route = false;
						con_lan.softap = true;
					}else{
						con_lan.route = false;
						con_lan.softap = false;
					};
					let res = await fetch('/set_lan', {
			      method: 'POST',
			      headers: {
					    'Content-Type': 'application/json;charset=utf-8'
					  },
			      body: JSON.stringify(con_lan)
			    });
			    if (res.status == 510){
			    	set_err(1);
			    	snd = false;
			    }else if(res.status == 200){
			    	if (alrt)	alert("Настройки сети удачно сохранены!");
			    	snd = true;
			    	if (cb) cb();
			    }else{
			    	alert("Что-то пошло не так!");
			    	snd = false;
			    };
				}else{
					snd = false;
				}
			break;
			case 3:
        now_j.channel=Number(get_el("i_n_k").value);
        const b_js = {...now_j};
        delete b_js.my_mac;
        for (let i = 0; i < now_j.max_peer; i++) {
          delete b_js.peers[i].rem;
          delete b_js.peers[i].cnf;
          delete b_js.peers[i].name;
        }
        fetch('/set_now', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json;charset=utf-8'
          },
          body: JSON.stringify(b_js)
        }).then(function(res){
          if(res.status == 200){
            if (alrt) alert("Серверы удачно сохранены!");
            snd = true;
            if (cb) cb();
          }else{
            alert("Что-то пошло не так!");
            snd = false;
          };
        });
			break;
			case 4:
				fD.append("ntpServer1", get_el("i_t_s1").value);
				fD.append("ntpServer2", get_el("i_t_s2").value);
				fD.append("ntpServer3", get_el("i_t_s3").value);
				fD.append("pereod_alarm", get_el("i_t_pt").value);
				fetch('/set_time', {
		      method: 'POST',
		      body: fD
		    }).then(function(res){
			    if(res.status == 200){
			    	if (alrt) alert("Настройки времени удачно сохранены!");
			    	snd = true;
			    	if (cb) cb();
			    }else{
			    	alert("Что-то пошло не так!");
			    	snd = false;
			    };
			  });
			break;
			case 5:
				if (serv_id >= 0)serv_up();
				fetch('/set_server', {
		      method: 'POST',
		      headers: {
				    'Content-Type': 'application/json;charset=utf-8'
				  },
		      body: JSON.stringify(servers)
		    }).then(function(res){
			    if(res.status == 200){
			    	if (alrt) alert("Серверы удачно сохранены!");
			    	snd = true;
			    	if (cb) cb();
			    }else{
			    	alert("Что-то пошло не так!");
			    	snd = false;
			    };
			  });
			break;
			case 6:
				if (dev_id >= 0)dev_up();
				fetch('/set_fingers', {
		      method: 'POST',
		      headers: {
				    'Content-Type': 'application/json;charset=utf-8'
				  },
		      body: JSON.stringify(devs)
		    }).then(function(res){
			    if(res.status == 200){
			    	if (alrt) alert("Станция удачно сохранена!");
			    	snd = true;
			    	if (cb) cb();
			    }else{
			    	alert("Что-то пошло не так!");
			    	snd = false;
			    };
			  });
			break;
			default:
			  snd = false;
			break;
		};
	} catch (error) {
	  console.log(error.message);
	  snd = false;
	};
	return snd;
}
function set_cl_t(mac, cnl){
	let fD = new FormData();
	peer_add.sts = 0;
	if (peer_add.tid) clearTimeout(peer_add.tid);
	peer_add.tid=null;
	peer_add.cnt=0;
	peer_add.mac_addr=mac;


	fD.append("mac", mac);
	fD.append("cnl", cnl);
	fetch("/add_me",{
    method: 'POST',
    body: fD
  }).then(res =>{
    if(res.status == 200){
    	get_sts(-1);
    	return res.text();
    }else if(res.status == 502){
    	alert("Занято предыдущей задачей!");
    	return "Занято предыдущей задачей!";
    }else{
    	alert("Что-то пошло не так!");
    	return "error";
    };
  }).then(function(res){
  	console.log(res);
  });
}
function ch_mac_peer(mac){
	for (let i = 0; i < now_j.peers.length; i++) {
		if ((mactostr(now_j.peers[i].mac_ap) == mac)&&(now_j.peers[i].act)){
			return i;
		}
	};
	return -1;
}
function cltbl(jsn){
	let tr;
	let tdl = get_el("tbl");
	let mac;
	while (tdl.rows.length>1) {
	  tdl.deleteRow(1);
	};
	for (let i = 0; i < jsn.length; i++) {
		mac = mactostr(jsn[i].bssid);
		let chkmac = ch_mac_peer(mac);
		tr=ce("tr");
		tr.insertCell(0).innerHTML = jsn[i].ssid;
		tr.insertCell(1).innerHTML = jsn[i].rssi;
		tr.insertCell(2).innerHTML = chkmac>=0?now_j.peers[i].name +" / "+mac:mac;
		tr.insertCell(3).innerHTML = jsn[i].channel;
		tr.insertCell(4).innerHTML = (jsn[i].hidden)?"Скрыта":"Видима";
		if (chkmac == -1) {
			tr.setAttribute("onclick", `set_cl_t("${mac}", ${jsn[i].channel})`);
			tr.classList.add("hov");
		}else{
			tr.style.textAlign = "center";
			tr.style.color = "#C33";
		}
		se(get_el("ap"), 0, tr);
	};
}
function sh_cnf(input){	
	let file = input.files[0];
	let reader = new FileReader();
	console.log("File name:", file.name);
	console.log("File name:", file.lastModified);
	reader.readAsText(file);
	reader.onload = function() {
		fetch('/cnf_set', {
      method: 'POST',
      headers: {
		    'Content-Type': 'application/json;charset=utf-8'
		  },
      body: reader.result
    }).then(function(res){
	    if(res.status == 200){
	    	alert("Конфиг удачно применен!");
	    }else{
	    	alert("Что-то пошло не так!");
	    };
	  }).catch(function(err){
	  	console.log(err);
	  	alert("Что-то пошло не так!");
	  });
  };
  reader.onerror = function() {
    console.log(reader.error);
  };
}
function serv_up(){
	servers[serv_id].act = get_el("chb_s").checked;
	servers[serv_id].host = get_el("i_s_ph").value;
	servers[serv_id].port = Number(get_el("i_s_pp").value);
}
function serv_cnf(id){
	if (serv_id >= 0)serv_up();
	get_el("n_tt").innerHTML = "Сервер "+(Number(id)+1);
	get_el("chb_s").checked = servers[id].act;
	get_el("i_s_ph").value = servers[id].host;
	get_el("i_s_pp").value = servers[id].port;
	serv_id=id;
}
function dev_up(){
	if (devs[dev_id].id){
		devs[dev_id].act = get_el("chb_e").checked;
		devs[dev_id].name = get_el("i_e_n").value;
		devs[dev_id].phalanx = Number(get_el("i_n_d").value);
	}else{
		let cnf_a = false;
		if (get_el("chb_e").checked != devs[dev_id].act){
			get_el("chb_e").checked = devs[dev_id].act;
			cnf_a = true;
		}
		if (get_el("i_e_n").value != devs[dev_id].name){
			get_el("i_e_n").value = devs[dev_id].name;
			cnf_a = true;
		}
		if (get_el("i_n_d").value != devs[dev_id].phalanx){
			get_el("i_n_d").value = devs[dev_id].phalanx;
			cnf_a = true;
		}
		if (cnf_a) alert("Нельзя редактировать не зарегестрированный отпечаток!");
	}
}
function dev_cnf(id){
	if (dev_id >= 0)dev_up();
	get_el("n_tt").innerHTML = "Отпечаток "+(Number(id)+1);
	get_el("chb_e").checked = devs[id].act;
	get_el("i_e_n").value = devs[id].name;
	get_el("i_n_d").value = devs[id].phalanx;
	dev_id=id;
}
function sts_fetch(id){
	if (id >= 0){
		fetch(`/sts_now?mac=${mactostr(now_j.peers[id].mac_addr)}`).then(res => res.json()).then(jsn => {
			now_j.peers[id].sts = jsn.sts;
			get_sts(id, jsn);
		}).catch(err => {
			if ((!now_j.peers[id].sts)&&(now_j.peers[id].cnt > max_re_sts)){
				now_j.peers[id].tid=null;
				if (now_j.peers[id].cmd == 7) get_el("chb_s_b").checked = !get_el("chb_s_b").checked;
				if (now_j.peers[id].cmd == 8) get_el("chb_s_w").checked = !get_el("chb_s_w").checked;
				now_j.peers[id].cmd = 0;
				console.log("Timeout!");
				alert("Время ответа истекло!");
			}else{
				if (!now_j.peers[id].sts) now_j.peers[id].cnt++;
				get_sts(id);
			}
		});
	}else{
		fetch(`/sts_now?mac=${peer_add.mac_addr}`).then(res => res.json()).then(jsn => {
			peer_add.sts = jsn.sts;
			get_sts(-1, jsn);
		}).catch(err => {
			if ((!peer_add.sts)&&(peer_add.cnt > max_re_sts)){
				peer_add.tid=null;
				console.log("Timeout!");
				alert("Время ответа истекло!");
			}else{
				if (!peer_add.sts) peer_add.cnt++;
				get_sts(id);
			}
		});
	}
}
function dev_reboot(id){
	let fD = new FormData();
	fD.append("mac", mactostr(now_j.peers[id].mac_addr));
	fetch('/res_now', {
    method: 'POST',
    body: fD
  }).then(function(res){
    if(res.status == 200){
    	set_var(id, 14);
    	get_sts(id);
    }else if(res.status == 502){
    	alert("Занято предыдущей задачей!");
    	console.log("Занято предыдущей задачей!");
    }else{
    	alert("Что-то пошло не так!");
    	console.log("err");
    };
  });
}
function device_viv(jsn, id){
	now_j.peers[id].rem = {...jsn};
	console.log(now_j.peers[id]);
	get_el("btn_psp").onclick = () =>{
		save_rem_peer(id);
	};
	get_el("btn_psn").onclick = () =>{
		save_rem_peer(id, 1)
	};
	get_el("btn_clr").onclick = () =>{
		clear_rem_peer(id);
	};
	get_el("pop2").style.display="block";
	get_el("i_n_n_p").textContent = now_j.peers[id].name;
	get_el("i_n_l_p").value = now_j.peers[id].lastname;
	get_el("i_n_i_p").value = now_j.peers[id].null_id;
	get_el("i_n_c_p").value = now_j.peers[id].cnt;
	get_el("chb_s_a").checked = now_j.peers[id].auto_measure;
	const blth_dev = get_el("chb_s_b");
	const wifi_dev = get_el("chb_s_w");
	blth_dev.checked = jsn.bluetooth;
	wifi_dev.checked = jsn.wifi;
	wifi_dev.dev = id;
	blth_dev.dev = id;
	wifi_dev.onclick=function(e){
		check_s(this);
	};
	blth_dev.onclick=function(e){
		check_s(this);
	};
	get_el("btn_prn").onclick=function(e){
		dev_reboot(id);
	};
	let tr, td, nms, i=0;
	let tbl=get_el("peer_pdev");
	while (tbl.rows.length>1) {
	  tbl.deleteRow(1);
	};
	const my_mac = mactostr(now_j.my_mac);
	jsn.peers.forEach(peer => {
		if (peer.act){
			tr=ce("tr");
			nms = mactostr(peer.mac_addr);
			if (my_mac == nms) nms = "Мой / "+nms;
			tr.insertCell(0).innerHTML = nms;
			tr.insertCell(1).innerHTML = peer.cnl;
			td=tr.insertCell(2);
			td.innerHTML = "Удалить";
			td.setAttribute("onclick", `del_peer_old(${id}, ${i})`);
			td.classList.add("hov");
			se(tbl, 0, tr);
			tr.style.textAlign = "center";
		}
		i++;
	});
}
function set_ext(){
	get_el("pop2").onclick = function(e){
		get_el("pop2").style.display="none";
	};
}
function check_add_phalanx(){
	fetch("/sts_finger")
	.catch(()=>{
		alert("Что-то пошло не так!");
		set_ext();
	})
	.then(res => {
		console.log(res);
		if (res.ok){
			res.json();
		}else{
			return {sts:4, res};
		}
	})
	.catch((e)=>{
		console.log(e);
		alert("Что-то пошло не так!");
		set_ext();
	})
	.then(jsn=>{
		switch (jsn.sts) {
			case 1:
				setTimeout(check_add_phalanx, 1000);
				break;	
			case 2:
				set_val(6);
				get_el("pop2").style.display="none";
				return "Палец добавлен! ID: "+jsn.id;
				break;
			case 3:
				set_ext();
				return "Что-то пошло не так!";
				break;	
			case 4:
				set_ext();
				return jsn.res.text();
				break;
			default:
				set_ext();
				return "Задание выполнено!";
				break;
		}
	}).then(txt =>{
		alert(txt);
	});
}
function add_phalanx(){
	get_el("btn_ssp").onclick = () =>{
		if (get_el("i_n_l_p").value.length>=3){
			let fD = new FormData();
			fD.append("nm", get_el("i_n_l_p").value);
			fD.append("ph", get_el("i_n_i_p").value);
			get_el("pop2").onclick = function(e){
				alert("Дождитесь завершения!");
		  };
			fetch("/addPhalanx", {
				method: "POST",
		    body: fD
			})
			.catch(()=>{
				alert("Что-то пошло не так!");
				set_ext();
			})
			.then(res => {
				if (res.ok){
					check_add_phalanx();
				}else{
					set_ext();
				}
				return res.text();
			}).then(txt=>{
				alert(txt);					
			});
		}else{
			alert("Име не менее 3 знаков!");
		}
	};
	get_el("pop2").style.display="block";

}
function get_sts(id, jsn=null){
	let b_obj = {};
	let sts;
	if (id >= 0) b_obj = now_j.peers[id];
	else b_obj = peer_add;
	if (jsn) sts = jsn.sts;
	else sts = b_obj.sts;
	if (sts){
		switch(sts){
			case 1:
				if (b_obj.cnt > max_re_sts){
					if (id >=0){
						if (now_j.peers[id].cmd == 7) get_el("chb_s_b").checked = !get_el("chb_s_b").checked;
						if (now_j.peers[id].cmd == 8) get_el("chb_s_w").checked = !get_el("chb_s_w").checked;
						now_j.peers[id].cmd = 0;
					}
					b_obj.sts=0;
					if (b_obj.tid){
						clearTimeout(b_obj.tid);
						b_obj.tid=null;
					}
					console.log("Timeout!");
					alert("Время ответа истекло!");
				}else{
					b_obj.cnt++;
					b_obj.tid = setTimeout(sts_fetch, 2000, id);
				}
			break;
			case 2:
				if (id >=0){
					if (now_j.peers[id].cmd == 7) get_el("chb_s_b").checked = !get_el("chb_s_b").checked;
					if (now_j.peers[id].cmd == 8) get_el("chb_s_w").checked = !get_el("chb_s_w").checked;
					now_j.peers[id].cmd=0;
				}
				b_obj.sts=0;
				b_obj.tid=null;
				alert("Ошибка!\nВозможно удаленное устройство не ответило!");
			break;
			case 3:
				b_obj.sts=0;
				b_obj.tid=null;
				switch(jsn.cmd){
					case 1:
						console.log("add me ok!");
						get_el("pop").style.display="none";
						set_val(2);
					break;
					case 2:
						console.log("del me ok!");
						set_val(2);
					break;
					case 3:
						console.log("del peer ok!");						
						fetch(`/rem_cnf?mac=${mactostr(b_obj.mac_addr)}`).then(res => res.json()).then(jsn =>device_viv(jsn, id));
					break;
					case 4:
						console.log("clear peers ok!");
					break;
					case 5:
						console.log("get peers ok!");
					break;
					case 6:
						console.log("save peers ok!");
						alert("Пиры сохранены!");
					break;
					case 7:
						alert(`Bluetooth ${get_el("chb_s_b").checked?"включен":"отключен"}`);
						console.log("ble_on_of ok!");
					break;
					case 8:
						alert(`Wifi ${get_el("chb_s_w").checked?"включен":"отключен"}`);
						console.log("wifi_on_of ok!");
					break;
					case 9:
						console.log("start get wif_ble!");
					break;
					case 10:
						console.log("start measure ok!");
					break;
					case 11:
						console.log("get config ok!");
						fetch(`/rem_cnf?mac=${mactostr(b_obj.mac_addr)}`).then(res => res.json()).then(jsn =>device_viv(jsn, id));
					break;
					case 12:
						console.log("set config ok!");
						alert("Конфиг сохранен!");
					break;
					case 13:
						console.log("set channel ok!");
						alert("Канал устройства изменен!");
					break;
					case 14:
						console.log("reboot ok!");
						alert("Удаленное устройство перезагружено!");
					break;
				}
			break;
		}
	}else{
		if (b_obj.tid) clearTimeout(b_obj.tid);
		b_obj.tid = setTimeout(sts_fetch, 3000, id);
	}
}
function set_var(id, cmd=0){
	now_j.peers[id].sts=0;
	if (now_j.peers[id].tid) clearTimeout(now_j.peers[id].tid);
	now_j.peers[id].tid=null;
	now_j.peers[id].cnt=0;
	now_j.peers[id].cmd=cmd;
}
function del_act_peer(id){
	let fD = new FormData();
	fD.append("mac", mactostr(act_j[id].mac_addr));
	fetch('/del_act', {
    method: 'POST',
    body: fD
  }).then(function(res){
    if(res.status == 200){
    	alert("Пир удалён!");
    }else{
    	alert("Что-то пошло не так!");
    };
  }).then(setTimeout(() =>{set_val(2);}, 5000)).catch(()=>alert("Что-то пошло не так!"));
}
function del_peer(id){
	let fD = new FormData();
	fD.append("mac", mactostr(now_j.peers[id].mac_addr));
	fetch('/del_me', {
    method: 'POST',
    body: fD
  }).then(function(res){
    if(res.status == 200){
    	set_var(id);
    	get_sts(id);
    }else if(res.status == 502){
    	alert("Занято предыдущей задачей!");
    	console.log("Занято предыдущей задачей!");
    }else{
    	alert("Что-то пошло не так!");
    	console.log("err");
    };
  });
}
function del_peer_old(id, id_o){
	let fD = new FormData();
	fD.append("mac", mactostr(now_j.peers[id].mac_addr));
	fD.append("mac_d", mactostr(now_j.peers[id].rem.peers[id_o].mac_addr));
	fetch('/del_old', {
    method: 'POST',
    body: fD
  }).then(function(res){
    if(res.status == 200){
    	set_var(id);
    	get_sts(id);
    }else if(res.status == 502){
    	alert("Занято предыдущей задачей!");
    	console.log("Занято предыдущей задачей!");
    }else{
    	alert("Что-то пошло не так!");
    	console.log("err");
    };
  });
}
function clear_rem_peer(id){
	let fD = new FormData();
	fD.append("mac", mactostr(now_j.peers[id].mac_addr));
	fetch('/clr_peer', {
    method: 'POST',
    body: fD
  }).then(function(res){
    if(res.status == 200){
    	set_var(id, 4);
    	get_sts(id);
    }else if(res.status == 502){
    	alert("Занято предыдущей задачей!");
    	console.log("Занято предыдущей задачей!");
    }else{
    	alert("Что-то пошло не так!");
    	console.log("err");
    };
  });
}
function save_rem_peer(id, tp=0){
	let fD = new FormData();
	switch(tp){
		case 0:
			fD.append("id", id);
			fD.append("null_id", get_el("i_n_i_p").value);
			fD.append("cnt", get_el("i_n_c_p").value);
			fD.append("lastname", get_el("i_n_l_p").value);
			fD.append("auto_measure", get_el("chb_s_a").checked);			
			fetch('/set_now_peer', {
		    method: 'POST',
		    body: fD
		  }).then(function(res){
		    if(res.status == 200){
		    	alert("Удачно сохранено!");
		    }else{
		    	alert("Что-то пошло не так!");
		    };
		  });
		break;
		case 1:
			now_j.peers[id].rem.bluetooth = get_el("chb_s_b").checked;
			now_j.peers[id].rem.wifi = get_el("chb_s_w").checked;
			let bf_obj = {...now_j.peers[id].rem};
			bf_obj.mac = [...now_j.peers[id].mac_addr];
			fetch('/rem_cnf', {
	      method: 'POST',
	      headers: {
			    'Content-Type': 'application/json;charset=utf-8'
			  },
	      body: JSON.stringify(bf_obj)
	    }).then(function(res){
		    if(res.status == 200){
		    	alert("Удачно сохранено!");
		    }else{
		    	alert("Что-то пошло не так!");
		    };
		  });
		break;
	}
}
function cnf_peer(id){
	if (now_j.peers[id].cnf){
		fetch(`/rem_cnf?mac=${mactostr(now_j.peers[id].mac_addr)}`).then(res => res.json()).then(jsn => device_viv(jsn, id));
	}else{
		fetch(`/get_now_cnf?mac=${mactostr(now_j.peers[id].mac_addr)}`).then(function(res){
	    if(res.status == 200){
	    	set_var(id);
	    	get_sts(id);
	    	return res.text();
	    }else if(res.status == 502){
	    	alert("Занято предыдущей задачей!");
	    	return "Занято предыдущей задачей!";
	    }else{
	    	alert("Что-то пошло не так!");
	    	return "error";
	    };
	  }).then(function(res){
	  	console.log(res);
	  });
	}
}
async function set_val(pp) {
	let vl;
	switch(pp){
		case 1:
			try {
				const res = await fetch("/lan.json");
				if (res.ok){
					con_lan = await res.json();
					get_el("i_l_n").value = con_lan.mdns_name;
					get_el("i_l_p").value = con_lan.pass_esp;
					get_el("i_l_n_r").value = con_lan.name;
					get_el("i_l_p_r").value = con_lan.pass;
					get_el("i_l_l").value = con_lan.login;
					if (now_j.new_dev) get_el("i_l_p_lo").value = con_lan.pass_autch;
					let wf = get_el("i_l_w");
					if(con_lan.route && con_lan.softap){
						wf.value = 3;
					}else if(con_lan.route){
						wf.value = 2;
					}else if(con_lan.softap){
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
				const res = await fetch("/now");
				if (res.ok){
					now_j = await res.json();
					get_el("i_n_k").value = now_j.channel;
					if (!now_j.new_dev){
						fetch("/peers").then(res => res.json()).then(jsn => {
							now_j.peers = [...jsn];
							fetch("/peers_act").then(res => res.json()).then(jsn_act => {
								act_j = [...jsn_act];
								let tr_a, td_a, mac;
								let tbl_a=get_el("peer_act");
								while (tbl_a.rows.length>1) {
								  tbl_a.deleteRow(1);
								};
								for (let i = 0; i < act_j.length; i++) {
									if (act_j[i].act){
										if ((act_j[i].cnf_id >=0 )&&(now_j.peers[act_j[i].cnf_id].name.length)&&(now_j.peers[act_j[i].cnf_id].act))
											mac = now_j.peers[act_j[i].cnf_id].name+" / "+mactostr(act_j[i].mac_addr);
										else
											mac = mactostr(act_j[i].mac_addr);
										tr_a=ce("tr");
										tr_a.insertCell(0).innerHTML = mac;
										td_a=tr_a.insertCell(1);
										td_a.innerHTML = "Удалить";
										td_a.setAttribute("onclick", `del_act_peer(${i})`);
										td_a.classList.add("hov");
										se(tbl_a, 0, tr_a);
										tr_a.style.textAlign = "center";				
									};
								};
							});
							let tr, td, nms;
							let tbl=get_el("peer");
							while (tbl.rows.length>1) {
							  tbl.deleteRow(1);
							};
							for (let i = 0; i < now_j.peers.length; i++){
								if (now_j.peers[i].act){
									if ((now_j.peers[i].lastname.length)&&(now_j.peers[i].name.length))
										nms = now_j.peers[i].name+" / "+now_j.peers[i].lastname;
									else
										if (now_j.peers[i].lastname.length) 
											nms = now_j.peers[i].lastname;
										else
											nms = now_j.peers[i].name;
									tr=ce("tr");
									tr.insertCell(0).innerHTML = nms;
									tr.insertCell(1).innerHTML = mactostr(now_j.peers[i].mac_addr);
									td=tr.insertCell(2);
									td.innerHTML = "Удалить";
									td.setAttribute("onclick", `del_peer(${i})`);
									td.classList.add("hov");
									td=tr.insertCell(3);
									td.innerHTML = "Настроить";
									td.setAttribute("onclick", `cnf_peer(${i})`);
									td.classList.add("hov");
									se(tbl, 0, tr);
									tr.style.textAlign = "center";
								};
							};
						});
					};
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 3:
			try {
				const res = await fetch("/time.json");
				if (res.ok){
					const vl = await res.json();
					get_el("i_t_s1").value = vl.ntpServer1;
					get_el("i_t_s2").value = vl.ntpServer2;
					get_el("i_t_s3").value = vl.ntpServer3;
					get_el("i_t_pt").value = vl.pereod_alarm;
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 4:
			try {
				const res = await fetch("/val");
				if (res.ok){
					const vl = await res.json();
          
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
				};
				if (timerId){
					clearTimeout(timerId);
					timerId = null;
				}
				timerId = setTimeout(set_val, re_val, 4);
			} catch (error) {
			  console.log(error.message);
				if (timerId){
					clearTimeout(timerId);
					timerId = null;
				}
			  timerId = setTimeout(set_val, re_val, 4);
			};
		break;
		case 5:
			try {
				const res = await fetch("/servers.json");
				if (res.ok){
					const vl = await res.json();
					let tbl=get_el("el_t");
					let tr, td;
					while (tbl.rows.length>1) {
					  tbl.deleteRow(1);
					};
					servers.length = 0;
					for (let i = 0; i < vl.serv.length; i++) {
						tr=ce("tr");
						se(tbl, 0, tr);
						td=tr.insertCell(0);
						td.innerHTML = "Сервер "+(i+1);
						td.setAttribute("onclick", "serv_cnf('"+i+"')");
						tr.classList.add("hov");
						servers.push(vl.serv[i]);
					};
					serv_cnf(0);
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 6:
			try {
				const res = await fetch("/finger.json");
				if (res.ok){
					const vl = await res.json();
					let tbl=get_el("el_t");
					let tr, td;
					while (tbl.rows.length>1) {
					  tbl.deleteRow(1);
					};
					devs.length = 0;
					for (let i = 0; i < vl.length; i++) {
						tr=ce("tr");
						se(tbl, 0, tr);
						td=tr.insertCell(0);
            if (vl[i].name.length){
              td.innerHTML = vl[i].name + "("+phalanxs[vl[i].phalanx]+")";
							if (!vl[i].act){
								td.style.color = "red";
							}
            }else{
              td.innerHTML = "Отпечаток "+(i+1);
            }						
						td.setAttribute("onclick", "dev_cnf('"+i+"')");
						tr.classList.add("hov");
						devs.push(vl[i]);
					};
					dev_cnf(0);
				};
			} catch (error) {
			  console.log(error.message);
			};
		break;
		case 7:
			fetch("/search_ap", {timeout: 5000}).then(res => res.text()).then(tx=>{
				console.log(tx);
				setTimeout(function(){
					fetch("/get_ap", {timeout: 5000}).then(res => res.json())
					.then(jsn=>cltbl(jsn));
				},10000);
			}).catch((e) => {
				console.log(e);
			});
		break;
	};
}
function check_s(e){
	let fD = new FormData();
	switch (e.value) {
		case "1":
		break;
		case "2":
			fD.append("mac", mactostr(now_j.peers[e.dev].mac_addr));
			fD.append("set", e.checked);
			fetch('/set_wifi', {
		    method: 'POST',
		    body: fD
		  }).then(function(res){
		    if(res.status == 200){
		    	set_var(e.dev, 8);
		    	get_sts(e.dev);
		    }else if(res.status == 502){
		    	e.checked = !e.checked;
		    	alert("Занято предыдущей задачей!");
		    	console.log("Занято предыдущей задачей!");
		    }else{
		    	e.checked = !e.checked;
		    	alert("Что-то пошло не так!");
		    	console.log("err");
		    };
		  });
			console.log("set wifi device");
		break;
		case "3":			
			fD.append("mac", mactostr(now_j.peers[e.dev].mac_addr));
			fD.append("set", e.checked);
			fetch('/set_ble', {
		    method: 'POST',
		    body: fD
		  }).then(function(res){
		    if(res.status == 200){
		    	set_var(e.dev, 7);
		    	get_sts(e.dev);
		    }else if(res.status == 502){
		    	e.checked = !e.checked;
		    	alert("Занято предыдущей задачей!");
		    	console.log("Занято предыдущей задачей!");
		    }else{
		    	e.checked = !e.checked;
		    	alert("Что-то пошло не так!");
		    	console.log("err");
		    };
		  });
			console.log("set bluetooth device");
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
async function go_pg(pg, frm = "", pp = 0, ftch=false) {
	try {
	rem_int();
	if (ftch)	{
		const res = await fetch("/"+pg);
		if (!res.ok) return;
	};
	if (ftch)	main.innerHTML = await res.text();
	else main.innerHTML = pg;
	let cont;
	if (frm.length > 0){
		if (ftch)	{
			const cont = await fetch("/"+frm);
			if (!cont.ok) return;
		};
	};
	if (frm.length > 0){
		if (ftch)	get_el("frm").innerHTML = await cont.text();
		else get_el("frm").innerHTML = frm;
	};
	switch(pp){
		case 1:
			set_cl_bk("nw_h", "none");
			set_cl("btn_n", 20, true);
			set_val(1);
		break;
		case 2:
			set_cl("btn_p", 6);
			set_cl("btn_n", 21, true);
			set_val(2);
		break;
		case 3:
			set_cl("btn_p",7);
			set_cl("btn_n", 22, true);
			set_val(3);
		break;
		case 4:
			serv_id=-1;
			set_cl("btn_p",8);
			set_cl("btn_n", 23, true);
			set_val(5);
		break;
		case 5:
			dev_id=-1;
			set_cl("btn_p",9);
			set_cl("btn_n", 11, true);
			set_val(6);
		break;
		case 6:
			await fetch("/config_ok",{
		      method: 'POST', body:"Config ok!"});
			now_j.new_dev=false;
			title.style.display = "block";
		case 54:
			set_cl("btn_v", 12, true);
			get_el("btn_gcnf").onclick = download_conf;
			get_el("btn_res").onclick = () => {
				fetch("/res", {
					method: "PUT"
				}).then(res => res.text()).then(txt=>{alert(txt);})
				.catch(res => res.text()).then(txt=>{alert(txt);});
			};
			get_el("btn_od").onclick = () => {
				fetch("/open", {
					method: "PUT"
				}).then(res => res.text()).then(txt=>{alert(txt);})
				.catch(res => res.text()).then(txt=>{alert(txt);});
			};
      sp_err = document.getElementById("ev_err");
      sp_t = document.getElementById("ev_tmp");
      sp_h = document.getElementById("ev_hud");
			set_val(4);				
		break;
		case 56:
			let opt = ce("option");
			opt.innerHTML = "Без WIFI";
			opt.value=0;
			se(get_el("i_l_w"), 0, opt);
			set_cl("btn_sv", 13, true);
			set_val(1);
			get_el("btn_lan").classList.add("act");
		break;
		case 57:			
			get_el("btn_now").classList.add("act");
			set_cl("btn_sv", 14, true);
			const id_opt = get_el("i_n_i_p");
			const cnt_opt = get_el("i_n_c_p");
			for (let i = 0; i < now_j.max_peer+1; i++) {
				let opt = ce("option");
				opt.value = i;
				opt.text = i;
				se(cnt_opt, 0, opt);
				if (i < now_j.max_peer){
					opt = ce("option");
					opt.value = i;
					opt.text = i;
					se(id_opt, 0, opt);
				};
			};			
			get_el("popn").onclick = function(e){
				e.stopPropagation();
			};
			get_el("pop").onclick = function(e){
			  get_el("pop").style.display="none";
		  };
		  get_el("popn2").onclick = function(e){
				e.stopPropagation();
			};
			get_el("pop2").onclick = function(e){
			  get_el("pop2").style.display="none";
		  };
			await set_val(2);
			get_el("btn_ps").onclick=function(e){
				set_val(7);
			};
			get_el("btn_avp").onclick=function(e){
				get_el("pop").style.display="block";
				fetch("/get_ap").then(res=>res.json())
			.then(jsn=>cltbl(jsn));
			};					
		break;
		case 58:
			get_el("btn_tim").classList.add("act");
			set_cl("btn_sv", 15, true);
			set_val(3);
		break;
		case 59:
			serv_id=-1;
			get_el("btn_serv").classList.add("act");
			set_val(5);
			set_cl("btn_spv", 16);
			set_cl("btn_sv", 17);
		break;
		case 60:
			dev_id=-1;
			get_el("btn_stan").classList.add("act");			
			get_el("btn_srem").onclick = () => {
				fetch("/delFingers", {
					method: "PUT"
				}).catch(()=>{
					alert("Что-то пошло не так!");
				}).then(res => {
					if (res.ok) set_val(6);
					return res.text();
				}).then(txt=>{					
					alert(txt);					
				});
			};
			get_el("btn_spr").onclick = () => {
				let fD = new FormData();
				fD.append("id", dev_id+1);
				fetch("/delPhalanx", {
					method: "POST",
		      body: fD
				}).catch(()=>{
					alert("Что-то пошло не так!");
				})
				.then(res => {
					if (res.ok) set_val(6);
					return res.text();
				}).then(txt=>{					
					alert(txt);					
				});
			};
			get_el("btn_sad").onclick = add_phalanx;
		  get_el("popn2").onclick = function(e){
				e.stopPropagation();
			};
			get_el("pop2").onclick = function(e){
			  get_el("pop2").style.display="none";
		  };
			phalanxs.forEach((el, i) => {
				const opt = ce("option");
				opt.value = i;
				opt.text = el;
				get_el("i_n_i_p").add(opt);
			});
			set_val(6);
			set_cl("btn_spv", 18);
			set_cl("btn_sv", 19);
		break;
		case 61:
			
		break;
		};
	} catch (error) {
	  console.log(error.message);
	};
}

function jamp_p(pp) {
	if (pp < 6) del_cl("men-btn", "act");
	if (pp){
		if (timerId){
			clearTimeout(timerId);
			timerId = null;
		};
	};
	if(peer_add.tid){
		clearTimeout(peer_add.tid);
		peer_add.tid=null;
	}
	if ((typeof now_j === "object")&&(Array.isArray(now_j.peers))){
		now_j.peers.forEach(peer => {
			if ((peer.tid)&&(peer.tid)&&(typeof peer.tid != "undefined")){
				clearTimeout(peer.tid);
				peer.tid=null;
			}
		})
	}
	switch(pp){
		case 0:
			go_pg(volh, "", 54);
		break;
		case 1:
			go_pg(lan_r, lan, 56);
		break;
		case 2:
			go_pg(now_r, now_1, 57);
		break;
		case 3:
			go_pg(tim_r, timh, 58);
		break;
		case 4:
			go_pg(serv_r, servh, 59);
		break;
		case 5:
			go_pg(stan_r, stanh, 60);
		break;
		case 11:
			send_val(6, false, function(){	
				go_pg(volh, "", 6);
			});
		break;
		case 12:
			set_val(4);
		break;
		case 13:
			send_val(2);
		break;
		case 14:
			send_val(3);
		break;
		case 15:
			send_val(4);
		break;
		case 16:
			serv_up();
		break;
		case 17:
			send_val(5);
		break;
		case 18:
			dev_up();
		break;
		case 19:
			send_val(6);
		break;
		case 20:
			send_val(2, false, function(){	
				go_pg(start_2, now_1, 2);
			});
		break;
		case 21:
			send_val(3, false, function(){	
				go_pg(start_3, timh, 3);
			});
		break;
		case 22:
			send_val(4, false, function(){	
				go_pg(start_4, servh, 4);
			});
		break;
		case 23:
			send_val(5, false, function(){	
				go_pg(start_5, stanh, 5);
			});
		break;
	};
}

async function init_p(json) {
	title = get_el("ttl");
	main = get_el("main");
	now_j = {...json};
  title.style.display = "block";
  go_pg(volh, "", 54);
}


function ready(){
	fetch("/now").then(res => res.json())
		.then(tt => init_p(tt));
	set_cl("btn_sts", 0);
	set_cl("btn_lan", 1);
	set_cl("btn_now", 2);
	set_cl("btn_tim", 3);
	set_cl("btn_serv", 4);
	set_cl("btn_stan", 5);
}

document.addEventListener("DOMContentLoaded", ready);