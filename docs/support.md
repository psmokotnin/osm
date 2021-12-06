---
# Feel free to add content and custom Front Matter to this file.
# To modify the layout, see https://jekyllrb.com/docs/themes/#overriding-theme-defaults

layout: default
title: User support. Open Sound Meter
---

<div class="pricing-header px-3 py-3 pb-md-4 mx-auto text-center pb-md-5">
    <h1 class="display-4">User support</h1>
        <p>
            My goal is to make the application as simple as possible. Please watch this short video and you'll know the features of Open Sound Meter.
        </p>
        <iframe width="560" height="315" src="https://www.youtube.com/embed/dY2n7jxCURk" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
        <p>
            Or download <a class="download pdf" href="https://github.com/psmokotnin/osm/releases/download/{{site.current_version}}/overview.pdf" target="_blank">PDF overview</a> with the same information. 
    </p>
    
    <h2 class="display-5 pt-md-5">Questions and issues</h2>
    <p class="lead">
        <p>
            If you still have a question or you need support — join the users community on Facebook.<br/>
            Feel free to write comments or direct messages to the page.<br/>
            <br/>
            If you want to suggest a new feature, write a message on Facebook or email me: <a href="mailto:p.smokotnin@gmail.com">p.smokotnin@gmail.com</a><br/><br/>
            <a href="/about#donate">Your donations</a> make me work faster :)
        </p>
    </p>
    <div style="width: 100%">
        <div class="fb-page" data-href="https://www.facebook.com/opensoundmeter/" data-tabs="timeline" data-width="" data-height="" data-small-header="false" data-adapt-container-width="true" data-hide-cover="false" data-show-facepile="true"><blockquote cite="https://www.facebook.com/opensoundmeter/" class="fb-xfbml-parse-ignore"><a href="https://www.facebook.com/opensoundmeter/">Open Sound Meter</a></blockquote></div>
    </div>
</div>
<script>
$('.download.pdf').click(function() {
    gtag('event', 'downloadpdf', {
        'os' : 'pdf'
    });
    return true;
});
</script>
<!--
<div class="container">

  <div class="card-deck mb-3 text-center" style="">
    <div class="card mb-4 shadow-sm">
      <div class="card-header">
        <h4 class="my-0 font-weight-normal">Contribute</h4>
      </div>
      <div class="card-body">
        <a type="button" target="_blank" class="btn btn-lg btn-block btn-outline-secondary" href="{{site.source_page}}">Github</a>
      </div>
    </div>
    
    <div class="card mb-4 shadow-sm">
      <div class="card-header">
        <h4 class="my-0 font-weight-normal">Thank you</h4>
      </div>
      <div class="card-body">
        <a type="button" class="btn btn-lg btn-block btn-success" href="{{site.payme_page}}">Donate</a>
      </div>
    </div>
    
    <div class="card mb-4 shadow-sm">
      <div class="card-header">
        <h4 class="my-0 font-weight-normal">Suggest features</h4>
      </div>
      <div class="card-body">
        <a type="button" target="_blank" class="btn btn-lg btn-block btn-outline-secondary" href="{{site.issues_page}}">Issues</a>
      </div>
    </div>
  </div>

</div>
-->