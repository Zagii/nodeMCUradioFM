
    const con=function()
    {
        console.log("connected");
        let o=document.getElementById("pol");
        o.textContent="on-line"
        o.style="color:lime";
        document.getElementById("plug").style.color="lime";
        document.getElementById("plugm").style.color="lime";
        W.getSekLbl();
        W.getStats();
    };
    const dc=function()
    {
    console.log("DC");
    let o=document.getElementById("pol");
        o.textContent="off-line"
        o.style="color:darkred";
        document.getElementById("plug").style.color="darkred";
        document.getElementById("plugm").style.color="darkred";
    delete W; 
    };
    const msg=function(j)
    {
        console.log(j);
        for (let k in j) {
            if (j.hasOwnProperty(k)) {
                console.log(k + " -> " + j[k]);
                if(["GEO","TEMP","CISN","DESZCZ"].indexOf(k)>=0)
                {
                    document.getElementById(k.toLowerCase()).innerHTML=j[k];
                }
                if(k.substr(0,7)=="SEKCJA/")
                {
                    setStan(k[7],j[k]);
                }
            }

        }
        if(j.hasOwnProperty("LBL"))
        {
                G.setLbl(j.LBL.id,j.LBL.lbl);
                document.getElementById("Lbl"+j.LBL.id).innerHTML=j.LBL.lbl;
        }
       
        if(j.hasOwnProperty("CZAS"))
        {
            G.setCzasS(j["CZAS"]);
            document.getElementById('godz').innerHTML=G.getGodz();
            document.getElementById('dX').innerHTML=G.getDtStr();
        }
        if(j.hasOwnProperty("SYSTIME"))
        {
            document.getElementById("SYSTIME").innerHTML="Czas od restartu: "+j.SYSTIME;
        }
        if(j.hasOwnProperty("TRYB"))
        { 
				let s=document.getElementById("trSw");
				let t=document.getElementById("tryb");
				if(j["TRYB"]=="m")
				{
					t.textContent="manual";
					s.className="fas fa-toggle-off w3-xlarge"
					s.style="color:darkred";

				}else
				{
					t.textContent="auto";
					s.className="fas fa-toggle-on w3-xlarge"
					s.style="color:lime";
				}
        }
        if(j.hasOwnProperty("SEKCJE"))
        { 
                setStany(j["SEKCJE"]);
        }
    };

   
    //let w=new wsConn(con.bind(this),dc.bind(this),msg.bind(this));
    

    //setInterval(w.sendtest.bind(w), 5000);
    
    document.addEventListener("DOMContentLoaded", function(event) {
	 
        
        debug=document.getElementById('deb');
        console.log("document On load");
        W=new wsConn(con,dc,msg);
        G=new global();
        W.begin(3);
        for(i=1;i<7;i++)
        {
            add(i);
            console.log("Dodaje: "+i);
        }
        
    });
    function setStany(sekcje)
    {
        for(i=1;i<7;i++)
        {
            setStan(i,sekcje&(1<<i));
        }
    }
    function setStan(nr,stan)
    {
        let s=document.getElementById("bs"+nr);
        let i=document.getElementById("bi"+nr);
        if(stan==0)
        {
            s.innerHTML="OFF";
            i.style.color="darkred";
        }else
        {
            s.innerHTML="ON";
            i.style.color="lime";
        }
    }
    function sendStan( nr)
    {
        
        let s=document.getElementById("bs"+nr);
        let w=0;
        if(s.innerHTML=="OFF")w=1;
    //  a(w<<nr);
       // let jsonOb={ "SEKCJA/"+nr+"\"", "msg":w };
        //let msg=JSON.stringify(jsonOb);
		let msg="{ \"SEKCJA/"+nr+"\":"+w+"}";
        console.log("sendStan msg="+msg );
    //  deb("sendStan msg="+msg);
        W.send(msg);   
    }
    function trybSwitch()
    {
        let t=document.getElementById("tryb");
        console.log(t.textContent);
        let jsonOb;
        if(t.textContent=="auto")
        {
            jsonOb={"TRYB": "m" };
        }else
        {
            jsonOb={ "TRYB":"a" };
        }
        let msg=JSON.stringify(jsonOb);
        console.log("sendStan msg="+msg );
        W.send(msg);   
    }
    function add(i) {
        let d = document.createElement("div"); 
        console.log("add: "+d);
        d.className="w3-third w3-section";
        d.innerHTML= "<button type=\"submit\" class=\"button button5\" id=\"b"+i+"\" onclick=\"sendStan("+i+")\">"+
                    " <small id=\"bs"+i+"\">OFF</small>"+           
                    " <p class=\"w3-wide\" id=\"Lbl"+i+"\" >Sekcja "+i+"\</p>"+
                    " <i class=\"fa fa-shower fa-5x\" style=\"color:darkred\" id=\"bi"+i+"\"></i></button>";     
        let foo = document.getElementById("sDiv");
        foo.appendChild(d);
    }
    